#line 2 "plain_alarm.h"

#include <ESP32Time.h>
#include <AUnitVerbose.h>
#include <Firebase_ESP_Client.h>

#include "../../src/modules/alarm/plainAlarm.h"
#include "../../src/modules/alarm/plainTriggerMemory.h"
#include "../../src/memory/memory_provider.h"
#include "../../src/utils/timeHelper.h"

using namespace aunit;
PlainTriggerMemory triggerMemoryToLoad;

class MockPlainAlarm : public PlainAlarm
{
public:
    MockPlainAlarm(TriggerCallback callback, MemoryProvider *provider, std::string prefix) : PlainAlarm(callback, provider, prefix) {}
    virtual ~MockPlainAlarm() {}
    virtual void saveTriggerToNVS(std::shared_ptr<Trigger> t) { PlainAlarm::saveTriggerToNVS(t); }
    virtual bool loadTriggerFromNVS(int index) { return PlainAlarm::loadTriggerFromNVS(index); }
};

class MockMemoryPA : public MemoryProvider
{
public:
    MockMemoryPA() : MemoryProvider(){};
    virtual ~MockMemoryPA(){};
    virtual void begin(){};
    virtual void end(){};
    virtual void init(String name = "vivarium"){};

    // Extern modules settings
    virtual void saveStruct(String key, const void *value, size_t len)
    {
        struct_len = len;
        usedKeyToSave = key;
        triggerMemory_hour = ((PlainTriggerMemory *)value)->hour;
        triggerMemory_minute = ((PlainTriggerMemory *)value)->minute;
        triggerMemory_key = String(((PlainTriggerMemory *)value)->key);
    };
    virtual bool loadStruct(String key, void *value, size_t len)
    {
        memcpy(value, &triggerMemoryToLoad, sizeof(PlainTriggerMemory));
        return true;
    };

    //Writes
    virtual void saveString(String key, String value){};
    virtual String loadString(String key, String defaultValue) { return defaultValue; };

    virtual void saveBool(String key, bool value){};
    virtual bool loadBool(String key, bool defaultValue) { return defaultValue; };

    virtual void saveInt(String key, uint32_t value){};
    virtual uint32_t loadInt(String key, uint32_t defaultValue) { return defaultValue; }

    virtual void saveFloat(String key, float value){};
    virtual float loadFloat(String key, float defaultValue) { return defaultValue; }

    virtual void saveDouble(String key, double value){};
    virtual float loadDouble(String key, double defaultValue) { return defaultValue; };

    virtual void removeKey(String key)
    {
        removedKey = key;
    };
    virtual void factoryReset(){};

    int getWriteCount() { return _writeCount; }
    int getBytesWritten() { return _bytesWritten; }

    int triggerMemory_hour;
    int triggerMemory_minute;
    String triggerMemory_key;

    int struct_len = 0;
    int load_struct_len = 0;

    String removedKey = "";
    String usedKeyToSave = "";
    bool savedValue = false;

private:
    void _incrementWrites() { _writeCount++; };
    void _incrementBytes(int bytes) { _bytesWritten += bytes; };
};

void plainAlarmMockCallback()
{
}

class TestPlainAlarmOnce : public TestOnce
{
protected:
    void setup() override
    {
        memory = new MockMemoryPA();
        alarm = new MockPlainAlarm(&plainAlarmMockCallback, memory, "prefix.");
        triggerMemoryToLoad.hour = 1;
        triggerMemoryToLoad.minute = 12;
        sprintf(triggerMemoryToLoad.key, "fkey");
    }

    void teardown() override
    {
        delete alarm;
        delete memory;
    }

public:
    MockPlainAlarm *alarm;
    MockMemoryPA *memory;
};

testF(TestPlainAlarmOnce, create_trigger)
{
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);

    assertEqual(alarm->getTriggersCount(), 0);
    alarm->createTrigger(256, "first");
    assertEqual(alarm->getTriggersCount(), 1);

    std::shared_ptr<Trigger> tt = alarm->getNextTrigger();
    assertTrue(tt != nullptr);

    int ti;
    assertTrue(alarm->getNextTriggerTime(&ti));
    assertEqual(ti, 256);

    std::shared_ptr<Trigger> t = alarm->getNextTrigger();
    assertEqual(256, getTime(t->hour, t->minute));
}

testF(TestPlainAlarmOnce, next_trigger_same_time)
{
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);

    assertEqual(alarm->getTriggersCount(), 0);
    alarm->createTrigger(256, "first");
    assertEqual(alarm->getTriggersCount(), 1);
    alarm->createTrigger(257, "second");
    int ti;
    assertTrue(alarm->getNextTriggerTime(&ti));
    assertEqual(ti, 257);

    std::shared_ptr<Trigger> t = alarm->getNextTrigger();
    assertEqual(257, getTime(t->hour, t->minute));
}

testF(TestPlainAlarmOnce, save_trigger_to_nvs)
{
    std::shared_ptr<Trigger> t = std::make_shared<Trigger>();
    t->firebaseKey = "firebaseKey";
    t->hour = 2;
    t->storageId = INVALID_MEMORY_ID;
    t->id = 0;
    t->minute = 0;
    alarm->saveTriggerToNVS(t);
    assertEqual(memory->struct_len, (int)sizeof(PlainTriggerMemory));
    assertEqual(t->storageId, 0);
    assertTrue(memory->triggerMemory_key == "firebaseKey");
}

testF(TestPlainAlarmOnce, save_trigger_to_full_nvs)
{
    for (int i = 0; i < MAX_TRIGGERS; i++)
    {
        alarm->getAvailableMemoryId();
    }
    std::shared_ptr<Trigger> t = std::make_shared<Trigger>();
    t->firebaseKey = "firebaseKey";
    t->hour = 2;
    t->storageId = INVALID_MEMORY_ID;
    t->id = 0;
    t->minute = 0;
    alarm->saveTriggerToNVS(t);
    assertEqual(memory->struct_len, 0);
    assertEqual(t->storageId, INVALID_MEMORY_ID);
}

testF(TestPlainAlarmOnce, create_trigger_memory)
{
    assertEqual(alarm->getTriggersCount(), 0);
    alarm->createTrigger(getTime(1, 0), "first");
    assertEqual(alarm->getTriggersCount(), 1);
    assertEqual(memory->triggerMemory_key, "first");
    assertEqual(memory->triggerMemory_minute, 0);
    assertEqual(memory->triggerMemory_hour, 1);
}

testF(TestPlainAlarmOnce, load_trigger_from_nvs)
{
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);

    assertEqual(alarm->getTriggersCount(), 0);
    alarm->loadTriggerFromNVS(0);
    assertEqual(alarm->getTriggersCount(), 1);
    std::shared_ptr<Trigger> t = alarm->getNextTrigger();

    assertEqual(t->firebaseKey, "fkey");
    assertEqual(t->storageId, 0);
}

testF(TestPlainAlarmOnce, load_trigger_from_nvs_second)
{
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);
    alarm->createTrigger(getTime(0, 30), "first");

    assertEqual(alarm->getTriggersCount(), 1);
    alarm->loadTriggerFromNVS(0);
    assertEqual(alarm->getTriggersCount(), 2);
    std::shared_ptr<Trigger> t = alarm->getNextTrigger();
    assertEqual(t->storageId, 0);
    assertEqual(t->minute, 12);
    assertEqual(t->hour, 1);
    assertEqual(t->firebaseKey, "fkey");
}

testF(TestPlainAlarmOnce, remove_trigger_memory)
{
    alarm->removeTrigger("first");
    assertEqual("", memory->removedKey);
    alarm->createTrigger(getTime(1, 0), "first");
    alarm->removeTrigger("first");
    assertEqual("prefix.0", memory->removedKey);
    assertEqual(alarm->getTriggersCount(), 0);
}

// testF(TestPlainAlarmOnce, create_net_triggers_from_json)
// {
// }

// testF(TestPlainAlarmOnce, parse_trigger_custom_value)
// {
// }

// testF(TestPlainAlarmOnce, create_trigger_from_json)
// {
// }

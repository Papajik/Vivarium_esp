#line 2 "payload_alarm.h"

#include <ESP32Time.h>
#include <AUnitVerbose.h>
#include <Firebase_ESP_Client.h>

#include "../../src/modules/alarm/payloadAlarm.h"
#include "../../src/modules/alarm/payloadTriggerMemory.h"
#include "../../src/memory/memory_provider.h"
#include "../../src/utils/timeHelper.h"

using namespace aunit;
PayloadTriggerMemory<int> payloadTriggerMemoryToLoad;

class MockPayloadAlarm : public PayloadAlarm<int>
{
public:
    MockPayloadAlarm(TriggerCallback callback, MemoryProvider *provider, std::string prefix) : PayloadAlarm<int>(callback, provider, prefix) {}
    virtual ~MockPayloadAlarm() {}
    virtual void saveTriggerToNVS(std::shared_ptr<PayloadTrigger<int>> t) { PayloadAlarm<int>::saveTriggerToNVS(t); }
    virtual bool loadTriggerFromNVS(int index) { return PayloadAlarm<int>::loadTriggerFromNVS(index); }
    virtual bool getPayloadFromJson(FirebaseJson *, int &payload)
    {
        payload = 23232;
        return true;
    }
    virtual bool getPayloadFromValue(String key, String value, int &payload)
    {
        payload = 2222;
        return true;
    }
};

class PayloadMockMemoryPA : public MemoryProvider
{
public:
    PayloadMockMemoryPA() : MemoryProvider(){};
    virtual ~PayloadMockMemoryPA(){};
    virtual void begin(){};
    virtual void end(){};
    virtual void init(String name = "vivarium"){};

    // Extern modules settings
    virtual void saveStruct(String key, const void *value, size_t len)
    {
        struct_len = len;
        usedKeyToSave = key;
        triggerMemory_hour = ((PayloadTriggerMemory<int> *)value)->hour;
        triggerMemory_minute = ((PayloadTriggerMemory<int> *)value)->minute;
        triggerMemory_payload = ((PayloadTriggerMemory<int> *)value)->payload;
        triggerMemory_key = String(((PayloadTriggerMemory<int> *)value)->key);
    };
    virtual bool loadStruct(String key, void *value, size_t len)
    {
        memcpy(value, &payloadTriggerMemoryToLoad, sizeof(PayloadTriggerMemory<int>));
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
    int triggerMemory_payload;

    int struct_len = 0;
    int load_struct_len = 0;

    String removedKey = "";
    String usedKeyToSave = "";
    bool savedValue = false;

private:
    void _incrementWrites() { _writeCount++; };
    void _incrementBytes(int bytes) { _bytesWritten += bytes; };
};

void payloadAlarmMockCallback()
{
}

class TestPayloadAlarmOnce : public TestOnce
{
protected:
    void setup() override
    {
        memory = new PayloadMockMemoryPA();
        alarm = new MockPayloadAlarm(&payloadAlarmMockCallback, memory, "prefix.");
        payloadTriggerMemoryToLoad.payload = 54445;
        payloadTriggerMemoryToLoad.hour = 1;
        payloadTriggerMemoryToLoad.minute = 12;
        sprintf(payloadTriggerMemoryToLoad.key, "fkey");
    }

    void teardown() override
    {
        delete alarm;
        delete memory;
    }

public:
    MockPayloadAlarm *alarm;
    PayloadMockMemoryPA *memory;
};

testF(TestPayloadAlarmOnce, create_trigger)
{
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);

    assertEqual(alarm->getTriggersCount(), 0);
    alarm->createTrigger(256, "first", 11223);
    assertEqual(alarm->getTriggersCount(), 1);

    std::shared_ptr<PayloadTrigger<int>> tt = alarm->getNextTrigger();
    assertTrue(tt != nullptr);

    int ti;
    assertTrue(alarm->getNextTriggerTime(&ti));
    assertEqual(ti, 256);

    std::shared_ptr<PayloadTrigger<int>> t = alarm->getNextTrigger();
    assertEqual(256, getTime(t->hour, t->minute));
    assertEqual(11223, t->payload);
}

testF(TestPayloadAlarmOnce, next_trigger_same_time)
{
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);

    assertEqual(alarm->getTriggersCount(), 0);
    alarm->createTrigger(256, "first", 123);
    assertEqual(alarm->getTriggersCount(), 1);
    alarm->createTrigger(257, "second", 456);
    int ti;
    assertTrue(alarm->getNextTriggerTime(&ti));
    assertEqual(ti, 257);

    std::shared_ptr<PayloadTrigger<int>> t = alarm->getNextTrigger();
    assertEqual(257, getTime(t->hour, t->minute));
}

testF(TestPayloadAlarmOnce, save_trigger_to_nvs)
{
    std::shared_ptr<PayloadTrigger<int>> t = std::make_shared<PayloadTrigger<int>>();
    t->firebaseKey = "firebaseKey";
    t->hour = 2;
    t->storageId = INVALID_MEMORY_ID;
    t->payload = 1111;
    t->id = 0;
    t->minute = 0;
    alarm->saveTriggerToNVS(t);
    assertEqual(memory->struct_len, (int)sizeof(PayloadTriggerMemory<int>));
    assertEqual(t->storageId, 0);
    assertTrue(memory->triggerMemory_key == "firebaseKey");
    assertEqual(memory->triggerMemory_payload, 1111);
}

testF(TestPayloadAlarmOnce, save_trigger_to_full_nvs)
{
    for (int i = 0; i < MAX_TRIGGERS; i++)
    {
        alarm->getAvailableMemoryId();
    }
    std::shared_ptr<PayloadTrigger<int>> t = std::make_shared<PayloadTrigger<int>>();
    t->firebaseKey = "firebaseKey";
    t->hour = 2;
    t->storageId = INVALID_MEMORY_ID;
    t->id = 0;
    t->minute = 0;
    alarm->saveTriggerToNVS(t);
    assertEqual(memory->struct_len, 0);
    assertEqual(t->storageId, INVALID_MEMORY_ID);
}

testF(TestPayloadAlarmOnce, create_trigger_memory)
{
    assertEqual(alarm->getTriggersCount(), 0);
    alarm->createTrigger(getTime(1, 0), "first", 123);
    assertEqual(alarm->getTriggersCount(), 1);
    assertEqual(memory->triggerMemory_key, "first");
    assertEqual(memory->triggerMemory_minute, 0);
    assertEqual(memory->triggerMemory_hour, 1);
    assertEqual(memory->triggerMemory_payload, 123);
}

testF(TestPayloadAlarmOnce, load_trigger_from_nvs)
{
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);

    assertEqual(alarm->getTriggersCount(), 0);
    alarm->loadTriggerFromNVS(0);
    assertEqual(alarm->getTriggersCount(), 1);
    std::shared_ptr<PayloadTrigger<int>> t = alarm->getNextTrigger();

    assertEqual(t->firebaseKey, "fkey");
    assertEqual(t->storageId, 0);
}

testF(TestPayloadAlarmOnce, load_trigger_from_nvs_second)
{
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);
    alarm->createTrigger(getTime(0, 30), "first", 123);

    assertEqual(alarm->getTriggersCount(), 1);
    alarm->loadTriggerFromNVS(0);
    assertEqual(alarm->getTriggersCount(), 2);
    std::shared_ptr<PayloadTrigger<int>> t = alarm->getNextTrigger();
    assertEqual(t->storageId, 0);
    assertEqual(t->minute, 12);
    assertEqual(t->hour, 1);
    assertEqual(t->payload, 54445);
    assertEqual(t->firebaseKey, "fkey");
}

testF(TestPayloadAlarmOnce, remove_trigger_memory)
{
    alarm->removeTrigger("first");
    assertEqual("", memory->removedKey);
    alarm->createTrigger(getTime(1, 0), "first", 123);
    alarm->removeTrigger("first");
    assertEqual("prefix.0", memory->removedKey);
    assertEqual(alarm->getTriggersCount(), 0);
}

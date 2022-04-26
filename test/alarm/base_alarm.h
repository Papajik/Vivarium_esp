#line 2 "base_alarm.h"

#include <ESP32Time.h>
#include <AUnitVerbose.h>
#include <Firebase_ESP_Client.h>

#include "../../src/modules/alarm/baseAlarm.h"
#include "../../src/utils/timeHelper.h"

using namespace aunit;

class MockAlarm : public BaseAlarm<Trigger>
{

public:
    MockAlarm(TriggerCallback callback, MemoryProvider *provider, std::string prefix) : BaseAlarm<Trigger>(callback, provider, prefix) {}
    virtual ~MockAlarm() {}
    void createTrigger(int time, String key) {}

    void insertTrigger(std::shared_ptr<Trigger> t)
    {
        _triggers.insert({t->firebaseKey, t});
    }

    String createTriggerKey = "";

protected:
    virtual void saveTriggerToNVS(std::shared_ptr<Trigger>) {}
    virtual void printTrigger(std::shared_ptr<Trigger>) {}
    virtual bool loadTriggerFromNVS(int index) { return false; }
    virtual void createNewTriggersFromJson(FirebaseJson *) {}
    virtual void parseTriggerCustomValue(std::shared_ptr<Trigger>, String jsonKey, String value) {}
    virtual bool updateTriggerFromJson(std::shared_ptr<Trigger> trigger, FirebaseJson *json) { return false; }
    virtual void createTriggerFromJson(FirebaseJson *json, String triggerKey) { createTriggerKey = triggerKey; }
};

void callback()
{
}

class TestBaseAlarmOnce : public TestOnce
{
protected:
    void setup() override
    {
        alarm = new MockAlarm(&callback, nullptr, "");
    }

    void teardown() override
    {
        delete alarm;
    }

public:
    MockAlarm *alarm;
};

testF(TestBaseAlarmOnce, parseTriggerJson)
{
    assertEqual("", alarm->createTriggerKey);
    FirebaseJson *json = new FirebaseJson();
    alarm->parseTriggerJson(json, "path/to/trigger/123");
    assertEqual("123", alarm->createTriggerKey);
    delete json;
}

testF(TestBaseAlarmOnce, get_next_trigger_nullptr)
{
    std::shared_ptr<Trigger> t = alarm->getNextTrigger();
    assertEqual(true, t == nullptr);
}

testF(TestBaseAlarmOnce, get_next_trigger)
{
    std::shared_ptr<Trigger> t = std::make_shared<Trigger>();
    t->firebaseKey = "first";
    t->hour = 10;
    t->minute = 15;
    t->storageId = 1;
    t->id = 1;
    alarm->insertTrigger(t);
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);
    std::shared_ptr<Trigger> r = alarm->getNextTrigger();
    assertEqual(r->firebaseKey, t->firebaseKey);
}

testF(TestBaseAlarmOnce, get_next_trigger_time_after)
{
    std::shared_ptr<Trigger> t = std::make_shared<Trigger>();
    t->firebaseKey = "first";
    t->hour = 10;
    t->minute = 15;
    t->storageId = 1;
    t->id = 1;
    alarm->insertTrigger(t);

    t = std::make_shared<Trigger>();
    t->firebaseKey = "second";
    t->hour = 10;
    t->minute = 30;
    t->storageId = 2;
    t->id = 2;
    alarm->insertTrigger(t);

    ESP32Time rtc;
    rtc.setTime(0, 25, 10, 1, 1, 2021);
    std::shared_ptr<Trigger> r = alarm->getNextTrigger();
    assertEqual(r->firebaseKey, t->firebaseKey);
}

testF(TestBaseAlarmOnce, get_next_trigger_time_beween)
{
    std::shared_ptr<Trigger> t = std::make_shared<Trigger>();
    t->firebaseKey = "first";
    t->hour = 10;
    t->minute = 15;
    t->storageId = 1;
    t->id = 1;
    alarm->insertTrigger(t);

    std::shared_ptr<Trigger> t2 = std::make_shared<Trigger>();
    t2->firebaseKey = "second";
    t2->hour = 10;
    t2->minute = 30;
    t2->storageId = 2;
    t2->id = 2;
    alarm->insertTrigger(t2);

    ESP32Time rtc;
    rtc.setTime(0, 35, 10, 1, 1, 2021);
    std::shared_ptr<Trigger> r = alarm->getNextTrigger();
    assertEqual(r->firebaseKey, t->firebaseKey);
}

testF(TestBaseAlarmOnce, clear_triggers)
{
    assertEqual(0, alarm->getTriggersCount());
    std::shared_ptr<Trigger> t = std::make_shared<Trigger>();
    t->firebaseKey = "first";
    t->hour = 10;
    t->minute = 15;
    t->storageId = 1;
    t->id = 1;
    alarm->insertTrigger(t);
    assertEqual(1, alarm->getTriggersCount());
    alarm->clearTriggers();
    assertEqual(0, alarm->getTriggersCount());
}

testF(TestBaseAlarmOnce, remove_trigger)
{
    assertEqual(0, alarm->getTriggersCount());
    std::shared_ptr<Trigger> t = std::make_shared<Trigger>();
    t->firebaseKey = "first";
    t->hour = 10;
    t->minute = 15;
    t->storageId = 1;
    t->id = 1;
    alarm->insertTrigger(t);
    t = std::make_shared<Trigger>();
    t->firebaseKey = "second";
    t->hour = 10;
    t->minute = 15;
    t->storageId = 2;
    t->id = 2;
    alarm->insertTrigger(t);
    assertEqual(2, alarm->getTriggersCount());
    alarm->removeTrigger("first");
    assertEqual(1, alarm->getTriggersCount());
    alarm->removeTrigger("first");
    assertEqual(1, alarm->getTriggersCount());
    alarm->removeTrigger("second");
    assertEqual(0, alarm->getTriggersCount());
}

testF(TestBaseAlarmOnce, find_trigger)
{
    std::shared_ptr<Trigger> n = alarm->findTrigger("unknown");
    assertTrue(n == nullptr);

    std::shared_ptr<Trigger> t = std::make_shared<Trigger>();
    t->firebaseKey = "first";
    t->hour = 10;
    t->minute = 15;
    t->storageId = 1;
    t->id = 112;
    alarm->insertTrigger(t);
    assertEqual(112, alarm->findTrigger("first")->id);
}

testF(TestBaseAlarmOnce, get_next_trigger_time)
{
    int ti;
    assertFalse(alarm->getNextTriggerTime(&ti));
    std::shared_ptr<Trigger> t = std::make_shared<Trigger>();
    t->firebaseKey = "first";
    t->hour = 10;
    t->minute = 15;
    t->storageId = 1;
    t->id = 112;
    alarm->insertTrigger(t);

    assertTrue(alarm->getNextTriggerTime(&ti));
    assertEqual(getTime(t->hour, t->minute), ti);
}

testF(TestBaseAlarmOnce, get_available_memory_id)
{

    int m;
    for (int i = 0; i < MAX_TRIGGERS; i++)
    {
        m = alarm->getAvailableMemoryId();
        assertEqual(m, i);
    }
    assertEqual(alarm->getAvailableMemoryId(), INVALID_MEMORY_ID);
    alarm->freeMemoryId(4);
    assertEqual(alarm->getAvailableMemoryId(), 4);
}

testF(TestBaseAlarmOnce, free_memory_id_trigger)
{
    for (int i = 0; i < 4; i++)
    {
        assertEqual(alarm->getAvailableMemoryId(), i);
    }
    std::shared_ptr<Trigger> t = std::make_shared<Trigger>();
    t->firebaseKey = "first";
    t->hour = 10;
    t->minute = 15;
    t->storageId = alarm->getAvailableMemoryId();
    t->id = 4;
    alarm->insertTrigger(t);
    for (int i = 5; i < MAX_TRIGGERS; i++)
    {
        assertEqual(alarm->getAvailableMemoryId(), i);
    }
    assertEqual(alarm->getAvailableMemoryId(), INVALID_MEMORY_ID);
    alarm->removeTrigger("first");
    assertEqual(alarm->getAvailableMemoryId(), 4);
}

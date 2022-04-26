#line 2 "test_feeder.h"
#include "../../src/modules/external/feeder/feeder.h"

#include <Firebase_ESP_Client.h>
#include <ESP32Time.h>
#include <AUnitVerbose.h>

using namespace aunit;

class TestFeederOnce : public TestOnce
{
protected:
    void setup() override
    {
        feederPtr = new Feeder(1, nullptr);
    }

    void teardown() override
    {
        delete feederPtr;
    }

public:
};

testF(TestFeederOnce, noTrigger)
{
    int time;
    assertFalse(feederPtr->getNextTriggerTime(&time));
    assertTrue(heap_caps_check_integrity_all(true));
}

testF(TestFeederOnce, triggersCount)
{
    FirebaseJson *json = new FirebaseJson();
    json->add("time", 256);
    assertEqual(0, feederPtr->getTriggersCount());
    feederPtr->parseJson(json, PREFIX_SETTINGS + String("/feeder/triggers/123"));
    assertEqual(1, feederPtr->getTriggersCount());
    feederPtr->parseJson(json, PREFIX_SETTINGS + String("/feeder/triggers/256"));
    assertEqual(2, feederPtr->getTriggersCount());
    feederPtr->parseValue(PREFIX_SETTINGS + String("/feeder/triggers/256"), "null");
    feederPtr->printTriggers();
    assertEqual(1, feederPtr->getTriggersCount());
    feederPtr->parseValue(PREFIX_SETTINGS + String("/feeder/triggers/123"), "null");
    assertEqual(0, feederPtr->getTriggersCount());

    delete json;
    assertTrue(heap_caps_check_integrity_all(true));
}

testF(TestFeederOnce, parseTrigger_afterCurrentTime)
{
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);

    FirebaseJson *json = new FirebaseJson();
    int wantedTime = 258;

    json->add("time", wantedTime);

    feederPtr->parseJson(json, PREFIX_SETTINGS + String("/feeder/triggers/123"));

    int time = 0;
    assertTrue(feederPtr->getNextTriggerTime(&time));
    assertEqual(time, wantedTime);

    delete json;
    assertTrue(heap_caps_check_integrity_all(true));
}

testF(TestFeederOnce, parseTrigger_beforeCurrentTime)
{
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);
    FirebaseJson *json = new FirebaseJson();
    int wantedTime = 108;

    json->add("time", wantedTime);

    feederPtr->parseJson(json, PREFIX_SETTINGS + String("/feeder/triggers/123"));

    int time = 0;
    assertTrue(feederPtr->getNextTriggerTime(&time));
    assertEqual(time, wantedTime);

    delete json;
    assertTrue(heap_caps_check_integrity_all(true));
}

testF(TestFeederOnce, _parseTwoTriggers_timeBetween)
{
    ESP32Time rtc;
    rtc.setTime(0, 1, 1, 1, 1, 2021);

    FirebaseJson *jsonBefore = new FirebaseJson();

    int beforeTime = 108;

    jsonBefore->add("time", beforeTime);

    FirebaseJson *jsonAfter = new FirebaseJson();

    int afterTime = 280;

    jsonAfter->add("time", afterTime);

    feederPtr->parseJson(jsonBefore, PREFIX_SETTINGS + String("/feeder/triggers/123"));
    feederPtr->parseJson(jsonAfter, PREFIX_SETTINGS + String("/feeder/triggers/456"));

    int time = 0;
    assertTrue(feederPtr->getNextTriggerTime(&time));
    assertEqual(time, afterTime);

    delete jsonBefore;
    delete jsonAfter;
    assertTrue(heap_caps_check_integrity_all(true));
}

testF(TestFeederOnce, notTriggered)
{
    feederPtr->setConnected(true, false);
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);

    FirebaseJson *json = new FirebaseJson();

    json->add("time", 258);

    feederPtr->parseJson(json, PREFIX_SETTINGS + String("/feeder/triggers/123"));

    assertEqual(feederPtr->getLastFeeded(), 0);

    Alarm.delay(10);

    assertEqual(feederPtr->getLastFeeded(), 0);

    rtc.setTime(0, 1, 1, 1, 1, 2021);
    Alarm.delay(10);
    assertEqual(feederPtr->getLastFeeded(), 0);
    rtc.setTime(0, 2, 1, 1, 1, 2021);
    Alarm.delay(10);
    assertEqual(feederPtr->getLastFeeded(), 258);
    assertTrue(heap_caps_check_integrity_all(true));
}
// Testing

// Next

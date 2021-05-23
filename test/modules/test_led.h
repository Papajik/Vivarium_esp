#include "../../src/modules/external/led/led.h"

#include <Firebase_ESP_Client.h>
#include <ESP32Time.h>
#include <AUnitVerbose.h>

using namespace aunit;

class TestLedOnce : public TestOnce
{
protected:
    void setup() override
    {
        ledModulePtr = new LedModule(1);
    }

    void teardown() override
    {
        delete ledModulePtr;
    }

public:
};

testF(TestLedOnce, defaultColor)
{
    assertEqual(ledModulePtr->getColor(), (uint32_t)0);
}

testF(TestLedOnce, setGetColor)
{
    ledModulePtr->setColor(2222);
    assertEqual(ledModulePtr->getColor(), (uint32_t)2222);
}

testF(TestLedOnce, noTrigger)
{
    uint32_t color;
    int time;
    assertFalse(ledModulePtr->getNextTriggerColor(&color));
    assertFalse(ledModulePtr->getNextTriggerTime(&time));
}

testF(TestLedOnce, triggersCount)
{
    FirebaseJson *json = new FirebaseJson();

    json->add("color", 123123);
    json->add("time", 256);
    assertEqual(0, ledModulePtr->getTriggersCount());
    ledModulePtr->parseJson(json, PREFIX_SETTINGS + String("/led/triggers/123"));
    assertEqual(1, ledModulePtr->getTriggersCount());
    ledModulePtr->parseJson(json, PREFIX_SETTINGS + String("/led/triggers/256"));
    assertEqual(2, ledModulePtr->getTriggersCount());
    ledModulePtr->parseValue(PREFIX_SETTINGS + String("/led/triggers/256"), "null");
    assertEqual(1, ledModulePtr->getTriggersCount());
    ledModulePtr->parseValue(PREFIX_SETTINGS + String("/led/triggers/123"), "null");
    assertEqual(0, ledModulePtr->getTriggersCount());

    delete json;
}

testF(TestLedOnce, parseTrigger_afterCurrentTime)
{
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);

    FirebaseJson *json = new FirebaseJson();
    uint32_t wantedColor = 123123;
    int wantedTime = 258;

    json->add("color", (int)wantedColor);
    json->add("time", wantedTime);

    ledModulePtr->parseJson(json, PREFIX_SETTINGS + String("/led/triggers/123"));

    uint32_t color = 0;
    int time = 0;
    assertTrue(ledModulePtr->getNextTriggerColor(&color));
    assertTrue(ledModulePtr->getNextTriggerTime(&time));
    assertEqual(time, wantedTime);
    assertEqual(color, wantedColor);

    delete json;
}

testF(TestLedOnce, parseTrigger_beforeCurrentTime)
{
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);
    FirebaseJson *json = new FirebaseJson();
    uint32_t wantedColor = 123123;
    int wantedTime = 108;

    json->add("color", (int)wantedColor);
    json->add("time", wantedTime);

    ledModulePtr->parseJson(json, PREFIX_SETTINGS + String("/led/triggers/123"));

    uint32_t color = 0;
    int time = 0;
    assertTrue(ledModulePtr->getNextTriggerColor(&color));
    assertTrue(ledModulePtr->getNextTriggerTime(&time));
    assertEqual(time, wantedTime);
    assertEqual(color, wantedColor);

    delete json;
}

testF(TestLedOnce, _parseTwoTriggers_timeBetween)
{
    ESP32Time rtc;
    rtc.setTime(0, 1, 1, 1, 1, 2021);

    FirebaseJson *jsonBefore = new FirebaseJson();

    uint32_t beforeColor = 25757;
    int beforeTime = 108;

    jsonBefore->add("color", (int)beforeColor);
    jsonBefore->add("time", beforeTime);

    FirebaseJson *jsonAfter = new FirebaseJson();

    uint32_t afterColor = 123123;
    int afterTime = 280;

    jsonAfter->add("color", (int)afterColor);
    jsonAfter->add("time", afterTime);

    ledModulePtr->parseJson(jsonBefore, PREFIX_SETTINGS + String("/led/triggers/123"));
    ledModulePtr->parseJson(jsonAfter, PREFIX_SETTINGS + String("/led/triggers/456"));

    uint32_t color = 0;
    int time = 0;
    assertTrue(ledModulePtr->getNextTriggerColor(&color));
    assertTrue(ledModulePtr->getNextTriggerTime(&time));
    assertEqual(time, afterTime);
    assertEqual(color, afterColor);

    delete jsonBefore;
    delete jsonAfter;
}

testF(TestLedOnce, notTriggered)
{
    ESP32Time rtc;
    rtc.setTime(0, 0, 1, 1, 1, 2021);

    FirebaseJson *json = new FirebaseJson();
    uint32_t color = 123123;

    json->add("color", (int)color);
    json->add("time", 258);

    ledModulePtr->parseJson(json, PREFIX_SETTINGS + String("/led/triggers/123"));

    assertEqual(ledModulePtr->getColor(), (uint32_t)0);

    Alarm.delay(10);

    assertEqual(ledModulePtr->getColor(), (uint32_t)0);

    rtc.setTime(0, 1, 1, 1, 1, 2021);
    Alarm.delay(10);
    assertEqual(ledModulePtr->getColor(), (uint32_t)0);
    rtc.setTime(0, 2, 1, 1, 1, 2021);
    Alarm.delay(10);
    assertEqual(ledModulePtr->getColor(), color);
}
// Testing

// Next

/**
* @file plainAlarm.cpp
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-10-24
* 
* @copyright Copyright (c) 2021
* 
*/
#include "plainAlarm.h"
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include "../../memory/memory_provider_internal.h"
#include <Firebase_ESP_Client.h>

#define JSON_STEP 2

struct PlainTriggerMemory
{
    int hour;
    int minute;
    char key[25];
};

PlainAlarm::PlainAlarm(TriggerCallback callback, MemoryProvider *provider, std::string prefix) : BaseAlarm<Trigger>(callback, provider, prefix) {}

void PlainAlarm::createTrigger(int time, String key)
{
    std::shared_ptr<Trigger> t = std::make_shared<Trigger>();
    t->hour = time / 256;
    t->minute = time % 256;
    t->firebaseKey = key;
    t->id = Alarm.alarmRepeat(t->hour, t->minute, 0, getCallback());
    lockSemaphore("createTrigger");
    _triggers.insert({key, t});
    unlockSemaphore();
    saveTriggerToNVS(t);
}

void PlainAlarm::saveTriggerToNVS(std::shared_ptr<Trigger> trigger)
{
    if (_provider == nullptr)
        return;

    if (trigger->storageId == INVALID_MEMORY_ID)
    {
        trigger->storageId = getAvailableMemoryId();
        if (trigger->storageId == INVALID_MEMORY_ID)
        {
            printlnE("Maximum of feed triggers reached");
            return;
        }
    }
    PlainTriggerMemory m;
    strcpy(m.key, trigger->firebaseKey.c_str());
    m.hour = trigger->hour;
    m.minute = trigger->minute;
    debugA("Size of trigger = %d", sizeof(PlainTriggerMemory));
    _provider->saveStruct(String(MAX_TRIGGERS) + String(trigger->storageId), &m, sizeof(PlainTriggerMemory));
}

void PlainAlarm::printTrigger(std::shared_ptr<Trigger> t)
{
    if (t != nullptr)
    {
        printI("ID: ");
        printI(t->id);
        printI("(fKey: ");
        printI(t->firebaseKey);
        printI(") --> ");
        printI(t->hour);
        printI(":");
        printlnI(t->minute);
    }
}

void PlainAlarm::loadTriggerFromNVS(int index)
{
    if (_provider == nullptr)
        return;

    PlainTriggerMemory enc;
    if (_provider->loadStruct(String(_memoryPrefix.c_str()) + String(index), &enc, sizeof(Trigger)))
    {
        auto t = std::make_shared<Trigger>();
        t->storageId = index;
        t->minute = enc.minute;
        t->hour = enc.hour;
        t->firebaseKey = String(enc.key);

        t->id = Alarm.alarmRepeat(t->hour, t->minute, 0, getCallback());
        lockSemaphore("loadTriggerFromNVS");
        _triggers.insert({t->firebaseKey, t});
        unlockSemaphore();
        printTrigger(t);
    }
}

void PlainAlarm::createNewTriggersFromJson(FirebaseJson *json)
{
    int type;
    String value;
    FirebaseJson innerJson;
    FirebaseJsonData innerData;

    int len = json->iteratorBegin();
    if (len % JSON_STEP == 0)
    {
        printlnA("Checking all triggers");
        for (int i = 0; i < json->iteratorBegin(); i += JSON_STEP)
        {
            String key;
            json->iteratorGet(i, type, key, value);
            lockSemaphore("parseTriggersJson2");
            auto it = _triggers.find(key);
            unlockSemaphore();

            // If key doesn't exists
            if (it == _triggers.end())
            {
                printlnA("Adding new trigger " + key);

                innerJson.setJsonData(value);

                // Get time
                if (innerJson.get(innerData, "/time"))
                {
                    createTrigger(innerData.intValue, key);
                }
            }
        }
    }
}

void PlainAlarm::parseTriggerCustomValue(std::shared_ptr<Trigger> trigger, String jsonKey, String value)
{
    printlnV("Editing " + jsonKey);

    if (jsonKey == "time")
    {
        trigger->hour = (int)value.toInt() / 256;
        trigger->minute = (int)value.toInt() % 256;
    }
}

bool PlainAlarm::updateTriggerFromJson(std::shared_ptr<Trigger> trigger, FirebaseJson *json)
{

    FirebaseJsonData data;
    bool changed = false;

    if (json->get(data, "/time"))
    {
        //Check time
        if (trigger->hour != data.intValue / 256)
        {
            trigger->hour = data.intValue / 256;
            changed = true;
        }
        if (trigger->minute != data.intValue % 256)
        {
            trigger->minute = data.intValue % 256;
            changed = true;
        }
    }
    return changed;
}

void PlainAlarm::createTriggerFromJson(FirebaseJson *json, String triggerKey)
{
    printlnV("Adding new trigger");

    FirebaseJsonData jsonData;
    if (json->get(jsonData, "/time", false))
    {

        createTrigger(jsonData.intValue, triggerKey);
    }
    else
    {
        printlnW("NO time in json");
        return;
    }
}
#include "feeder.h"

#include <vector>
#define KEY_TRIGGERS "/feeder/triggers"
#define KEY_MODE "/feeder/mode"

String Feeder::getSettingKey() {return SETTINGS_FEEDER_KEY; }

void Feeder::updateSensorData(FirebaseJson *)
{
}

void Feeder::parseJson(FirebaseJson *json, String path)
{
    printlnA("Feeder - parseJson");
    printlnV("Path: " + path);

    if (path.startsWith(PREFIX_SETTINGS + String(KEY_TRIGGERS)))
    {
        parseTriggerJson(json, path);
        printTriggers();
        return;
    }

    FirebaseJsonData jsonData;

    if (json->get(jsonData, KEY_TRIGGERS, false))
    {
        printlnV("");
        FirebaseJson triggersJson;
        if (jsonData.getJSON(triggersJson))
        {
            parseTriggersJson(&triggersJson);
        }
    }

    if (json->get(jsonData, KEY_MODE, false))
    {
        switch (jsonData.intValue)
        {
        case BOX:
            setMode(BOX);
            break;
        case SCREW:
            setMode(SCREW);
            break;
        default:
            setMode(INVALID);
            break;
        }
    }

    if (json->get(jsonData, FIREBASE_FEEDER_CONNECTED_KEY, false))
    {
        setConnected(jsonData.boolValue, false);
    }
}

void Feeder::parseTriggerJson(FirebaseJson *json, String path)
{
    FirebaseJsonData jsonData;
    printlnA("Feeder  parseTriggerJson");
    int index = path.lastIndexOf("/");
    String triggerKey = path.substring(index + 1);

    printlnV("Parse 1 trigger json callback");
    printlnV("TriggerKey = " + triggerKey);
    if (_triggers.find(triggerKey) == _triggers.end())
    {
        auto t = std::make_shared<FeedTrigger>();
        if (json->get(jsonData, "/time", false))
        {
            createTrigger(jsonData.intValue, triggerKey);
        }
    }
}

void Feeder::parseValue(String key, String value)
{
    printlnD("Feeder parse value");
    printV("key = ");
    printlnV(key);
    printV("value = ");
    printlnV(value);
    if (key.startsWith(PREFIX_SETTINGS + String(KEY_TRIGGERS)))
    {
        parseTriggerValue(key, value);
    }

    if (key == String(PREFIX_STATE) + FIREBASE_FEEDER_CONNECTED_KEY)
    {
        setConnected(value == "true", false);
    }
}

void Feeder::parseTriggerValue(String key, String value)
{
    printlnA("Parsing trigger value");
    int index = key.lastIndexOf("/");

    String jsonKey = key.substring(index + 1);
    String triggerKey = key.substring(0, index);
    triggerKey = triggerKey.substring(triggerKey.lastIndexOf("/") + 1);

    printlnV("jsonKey =" + jsonKey);
    printlnV("triggerKey = " + triggerKey);

    if (value == "null")
    {
        removeTrigger(jsonKey);
    }
    else
    {
        auto it = _triggers.find(triggerKey);
        if (it != _triggers.end())
        {

            printlnV("Editing " + triggerKey);
            Alarm.free(it->second->id);

            if (jsonKey == "time")
            {
                it->second->hour = (int)value.toInt() / 256;
                it->second->minute = (int)value.toInt() % 256;
            }
            it->second->id = Alarm.alarmRepeat(it->second->hour, it->second->minute, 0, feederCallback);
            saveTriggerToNVS(it->second);
        }
    }
    printTriggers();
}

void Feeder::parseTriggersJson(FirebaseJson *json)
{
    int type;
    String value;
    String firebaseId;

    FirebaseJsonData innerData;
    FirebaseJson innerJson;
    std::vector<String> toDelete;

    printlnA("FeederParsing triggers json");

    // 1. check all existing triggers;
    for (auto &&it : _triggers)
    {
        printlnA("Checking key = " + it.first);
        FirebaseJsonData triggerData;
        if (json->get(triggerData, "/" + it.first, true))
        {
            bool triggerChanged = false;

            // Edit existing trigger
            triggerData.getJSON(innerJson);
            if (innerJson.get(innerData, "/time"))
            {
                //Check time
                int time = innerData.intValue;
                triggerChanged = parseTime(it.second, time);


                // Reset timer if needed
                if (triggerChanged)
                {
                    printlnA("Trigger " + it.second->firebaseKey + " changed, reseting timer");
                    Alarm.free(it.second->id);
                    it.second->id = Alarm.alarmRepeat(it.second->hour, it.second->minute, 0, feederCallback);
                    saveTriggerToNVS(it.second);
                }
            }
        }
        else
        {
             printlnA("Deleting trigger " + it.second->firebaseKey);
            toDelete.push_back(it.second->firebaseKey);
        }
    }

    // Delete all marked triggers
    for (String s : toDelete)
    {
        removeTrigger(s);
    }

    // 2. check all new triggers
    int len = json->iteratorBegin();
    if (len % 2 == 0)
    {
        printlnA("Checking all triggers");
        for (int i = 0; i < json->iteratorBegin(); i += 2)
        {
            String key;
            json->iteratorGet(i, type, key, value);
            auto it = _triggers.find(key);

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
    printTriggers();
}
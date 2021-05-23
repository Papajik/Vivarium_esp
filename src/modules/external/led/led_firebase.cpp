#include "led.h"

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#define KEY_COLOR "/led/color"
#define KEY_TRIGGERS "/led/triggers"
#define KEY_SENSOR_DATA_COLOR "led/color"

/// Firebase
void LedModule::parseJson(FirebaseJson *data, String path)
{
    printlnI("LED - parseJson");
    printlnV("Path: " + path);

    FirebaseJsonData jsonData;

    if (path.startsWith(PREFIX_SETTINGS + String(KEY_TRIGGERS)))
    {
        parseTriggerJson(data, path);
        return;
    }

    if (data->get(jsonData, KEY_TRIGGERS, false))
    {
        printlnD("Parsing led triggers");
        printlnV(jsonData.type);

        FirebaseJson json;
        if (jsonData.getJSON(json))
        {
            parseTriggersJson(&json);
        }
        else
        {
            printlnE("PROBLEM");
        }
    }

    if (data->get(jsonData, KEY_COLOR, false))
    {
        setColor(jsonData.intValue);
    }

    if (_settingsChanged)
    {
        saveSettings();
    }

    if (data->get(jsonData, FIREBASE_LED_CONNECTED_KEY, false))
    {
        setConnected(jsonData.boolValue, false);
    }
    //   printTriggers();
}

String LedModule::getSettingKey() { return SETTINGS_LED_KEY; }

void LedModule::parseValue(String key, String value)
{
    printlnD("LED parse value");
    printV("key = ");
    printlnV(key);
    printV("value = ");
    printlnV(value);

    /// Led triggers
    if (key.startsWith(PREFIX_SETTINGS + String(KEY_TRIGGERS)))
    {
        parseTriggerValue(key, value);
    }

    //  Current color
    if (key == String(PREFIX_STATE) + KEY_COLOR)
    {

        setColor(value.toInt());
    }

    // led connected
    if (key == String(PREFIX_STATE) + FIREBASE_LED_CONNECTED_KEY)
    {
        setConnected(value == "true", false);
    }

    if (_settingsChanged)
    {
        saveSettings();
    }
    // printTriggers();
}

void LedModule::updateSensorData(FirebaseJson *json)
{
    if (isConnected())
    {
        json->set(KEY_SENSOR_DATA_COLOR, (int)_currentColor);
    }
}

void LedModule::parseTriggersJson(FirebaseJson *json)
{
    printlnV("Iterating through json");

    int type;
    String key;
    String value;
    String firebaseId;

    FirebaseJsonData innerData;
    FirebaseJson innerJson;
    std::vector<String> toDelete;
    // bool changed = false;

    // 1. check all existing triggers;

    for (auto &&it : _triggers)
    {
        FirebaseJsonData triggerData;
        bool triggerChanged = false;
        // Edit existing trigger
        triggerData.getJSON(innerJson);

        if (json->get(triggerData, "/" + it.first, true))
        {
            int _oldColor = it.second->color;

            if (innerJson.get(triggerData, "/color", false))
            {
                if (it.second->color != triggerData.intValue)
                {
                    it.second->color = triggerData.intValue;
                    triggerChanged = true;
                }
            }
            else
            {
                continue;
            }

            if (innerJson.get(innerData, "/time"))
            {
                //Check time
                if (it.second->hour != innerData.intValue / 256)
                {
                    it.second->hour = innerData.intValue / 256;
                    triggerChanged = true;
                }
                if (it.second->minute != innerData.intValue % 256)
                {
                    it.second->minute = innerData.intValue % 256;
                    triggerChanged = true;
                }
            }
            else
            {
                it.second->color = _oldColor;
                continue;
            }

            if (triggerChanged)
            {
                printlnA("Trigger " + key + " changed, reseting timer");
                Alarm.free(it.second->id);
                it.second->id = Alarm.alarmRepeat(it.second->hour, it.second->minute, 0, ledTriggerCallback);
                saveTriggerToNVS(it.second);
            }
        }
        else
        {
            printlnA("Deleting trigger " + key);
            toDelete.push_back(it.first);
        }
    }

    // Delete all marked triggers
    for (String s : toDelete)
    {
        removeTrigger(s);
    }

    //check for new triggers
    int len = json->iteratorBegin();
    if (len % 3 == 0)
    {
        printlnV("JSON Valid");
        for (int i = 0; i < len; i += 3)
        {

            json->iteratorGet(i, type, key, value);
            auto it = _triggers.find(key);
            if (it == _triggers.end()) // Check if map contain this key
            {
                if (type == FirebaseJson::JSON_OBJECT)
                {
                    auto trigger = std::make_shared<LedTrigger>();
                    innerJson.setJsonData(value);

                    if (innerJson.get(innerData, "/time", false))
                    {
                        printlnV("TriggerData = " + String(innerData.intValue));
                        trigger->hour = innerData.intValue / 256;
                        trigger->minute = innerData.intValue % 256;
                    }
                    else
                    {
                        printlnW("No time in json");
                        continue;
                    }

                    if (innerJson.get(innerData, "/color", false))
                    {
                        trigger->color = innerData.intValue;
                    }
                    else
                    {
                        printlnW("no color in json");
                        continue;
                    }

                    printlnV("inserted trigger");
                    trigger->id = Alarm.alarmRepeat(trigger->hour, trigger->minute, 0, ledTriggerCallback);
                    trigger->firebaseKey = key;
                    _triggers.insert({key, trigger});
                    saveTriggerToNVS(trigger);
                }
            }
        }
    }
    printTriggers();
}
void LedModule::parseTriggerJson(FirebaseJson *json, String path)
{
    FirebaseJsonData jsonData;
    printlnI("LED  parseTriggerJson");
    int index = path.lastIndexOf("/");
    String triggerKey = path.substring(index + 1);

    printlnV("Parse 1 trigger json callback");
    printlnV("TriggerKey = " + triggerKey);

    // adding new trigger;
    if (_triggers.find(triggerKey) == _triggers.end())
    {
        printlnV("Adding new trigger");
        auto trigger = std::make_shared<LedTrigger>();
        if (json->get(jsonData, "/color", false))
        {

            trigger->color = jsonData.intValue;
            printlnV(trigger->color);
        }
        else
        {
            printlnV("No color in json");
            return;
        }
        if (json->get(jsonData, "/time", false))
        {
            trigger->hour = jsonData.intValue / 256;
            trigger->minute = jsonData.intValue % 256;
        }
        else
        {
            printlnW("NO time in json");
            return;
        }

        trigger->id = Alarm.alarmRepeat(trigger->hour, trigger->minute, 0, ledTriggerCallback);
        printlnV("Adding new trigger");

        trigger->firebaseKey = triggerKey;
        _triggers.insert({triggerKey, trigger});
        saveTriggerToNVS(trigger);
    }
    printTriggers();
}

void LedModule::parseTriggerValue(String key, String value)
{
    printlnV("Parsing trigger value");
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
            if (jsonKey == "color")
            {
                it->second->color = value.toInt();
            }
            if (jsonKey == "time")
            {
                it->second->hour = (int)value.toInt() / 256;
                it->second->minute = (int)value.toInt() % 256;
            }
            it->second->id = Alarm.alarmRepeat(it->second->hour, it->second->minute, 0, ledTriggerCallback);

            saveTriggerToNVS(it->second);
        }
    }
    printTriggers();
}
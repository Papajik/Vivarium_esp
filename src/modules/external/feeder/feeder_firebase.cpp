#include "feeder.h"

#include <vector>
#define KEY_TRIGGERS "/feeder/triggers"
#define KEY_MODE "/feeder/mode"

String Feeder::getSettingKey() { return SETTINGS_FEEDER_KEY; }

bool Feeder::updateSensorData(FirebaseJson *)
{
    return false;
}

void Feeder::parseJson(FirebaseJson *json, String path)
{
    printlnA("Feeder - parseJson");
    printlnV("Path: " + path);

    if (path.startsWith(PREFIX_SETTINGS + String(KEY_TRIGGERS)))
    {
        parseTriggerJson(json, path);
        // printTriggers();
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
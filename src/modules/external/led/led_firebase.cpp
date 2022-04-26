#include "led.h"

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include "../../../utils/timeHelper.h"

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
}

bool LedModule::updateSensorData(FirebaseJson *json)
{
    if (isConnected())
    {
        json->set(KEY_SENSOR_DATA_COLOR, (int)_currentColor);
        return true;
    }
    return false;
}

bool LedModule::getPayloadFromJson(FirebaseJson *json, uint32_t &payload)
{
    printlnD("getPayloadFromJson");
    FirebaseJsonData data;
    if (json->get(data, "/color", false))
    {
        printD("got color: ");
        payload = data.doubleValue;
        printlnD(payload);
        return true;
    }
    else
    {
        printlnW("No color in JSON");
        return false;
    }
}

bool LedModule::getPayloadFromValue(String key, String value, uint32_t &payload)
{
    printlnD("getPayloadFromJson");
    if (key == "color")
    {
        printD("got color: ");
        payload = value.toInt();
        printlnD(payload);
        return true;
    }
    return false;
}

#include "fan.h"

#define KEY_SPEED "fan/speed"
#include <Firebase_ESP_Client.h>

#define KEY_START_AT "/fan/startAt"
#define KEY_MAX_AT "/fan/maxAt"

void FanController::parseJson(FirebaseJson *data, String path)
{
    printlnA("Fan - parseJson");
    FirebaseJsonData jsonData;

    if (data->get(jsonData, KEY_START_AT, false))
    {
        printlnA("SAVING start at");
        setStartAt(jsonData.floatValue);
    }

    if (data->get(jsonData, KEY_MAX_AT, false))
    {
        printlnA("SAVING max at");
        setMaxAt(jsonData.floatValue);
    }

    if (data->get(jsonData, FIREBASE_FAN_CONNECTED_KEY, false))
    {
        setConnected(jsonData.boolValue, false);
    }

    if (_settingsChanged)
    {
        saveSettings();
    }
}

void FanController::parseValue(String key, String value)
{
    printlnA("Fan parse value");
    printD("key = ");
    printlnD(key);
    printD("value = ");
    printlnD(value);

    if (key == String(PREFIX_SETTINGS) + KEY_START_AT)
    {
        setStartAt(value.toFloat());
    }

    if (key == String(PREFIX_SETTINGS) + KEY_MAX_AT)
    {
        setMaxAt(value.toFloat());
    }

    if (key == String(PREFIX_STATE) + FIREBASE_FAN_CONNECTED_KEY)
    {
        setConnected(value == "true", false);
    }

    if (_settingsChanged)
    {
        saveSettings();
    }
}

void FanController::updateSensorData(FirebaseJson *json)
{
   // printlnD("update fan data");
    if (isConnected())
    {
        printlnD("Current speed = ");
        printlnD(_currentSpeed);
        float tmp = float(_currentSpeed)/float(FAN_MAX_SPEED)*100;
        printlnD(tmp);
        int percentage = int(tmp);

        printD("Percentage = ");
        printlnD(percentage);
        json->set(KEY_SPEED, percentage);
    }
}

String FanController::getSettingKey() { return SETTINGS_FAN_KEY; }
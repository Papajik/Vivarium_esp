#include "heater.h"
#define KEY_SENSOR_DATA_HEATER_POWER "heater/power"
#define KEY_SENSOR_DATA_HEATER_TEMP_GOAL "heater/tempGoal"
#define KEY_HEATER_MODE "/heater/mode"
#define KEY_HEATER_GOAL "/heater/tempGoal"

void Heater::parseJson(FirebaseJson *json, String path)
{
    printlnA("Heater- parseJson");
    FirebaseJsonData jsonData;

    if (json->get(jsonData, KEY_HEATER_MODE, false))
    {
        switch (jsonData.intValue)
        {
        case PID:
            setMode(PID);
            break;
        case AUTO:
            setMode(AUTO);
            break;
        default:
            printlnW("UNKNOWN HEATER MODE - " + jsonData.intValue);
        }
    }

    if (json->get(jsonData, KEY_HEATER_GOAL, false))
    {
        printlnA("SAVING goal");
        setGoal(jsonData.doubleValue);
    }

    if (json->get(jsonData, FIREBASE_HEATER_CONNECTED_KEY, false))
    {
        setConnected(jsonData.boolValue, false);
    }

    if (_settingsChanged)
    {
        saveSettings();
    }
}

String Heater::getSettingKey() {printlnA("HEATER - settings key"); return SETTINGS_HEATER_KEY; }
void Heater::parseValue(String key, String value)
{
    printlnA("Heater parse value");
    printD("key = ");
    printlnD(key);
    printD("value = ");
    printlnD(value);

    if (key == String(PREFIX_SETTINGS) + KEY_HEATER_MODE)
    {
        switch (value.toInt())
        {
        case PID:
            setMode(PID);
            break;
        case AUTO:
            setMode(AUTO);
            break;
        default:
            printlnW("UNKNOWN HEATER MODE - " + value.toInt());
        }
    }

    if (key == String(PREFIX_SETTINGS) + KEY_HEATER_GOAL)
    {
        printlnA("Saving goal");
        setGoal(value.toDouble());
    }

    if (key == String(PREFIX_STATE) + FIREBASE_HEATER_CONNECTED_KEY)
    {
        setConnected(value == "true", false);
    }
}
void Heater::updateSensorData(FirebaseJson *json)
{
    if (isConnected())
    {
        json->set(KEY_SENSOR_DATA_HEATER_POWER, _currentPower);
        json->set(KEY_SENSOR_DATA_HEATER_TEMP_GOAL, _settings.tempGoal);
    }
}
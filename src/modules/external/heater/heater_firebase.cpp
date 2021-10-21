#include "heater.h"
#define KEY_SENSOR_DATA_HEATER_POWER "heater/power"
#define KEY_SENSOR_DATA_HEATER_TEMP_GOAL "heater/goal"
#define KEY_HEATER_MODE "/heater/mode"

#define KEY_TRIGGERS "/heater/triggers"

void Heater::parseJson(FirebaseJson *json, String path)
{
    printlnA("Heater - parseJson");
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
        case THERMO:
            setMode(THERMO);
            break;
        default:
            printlnW("UNKNOWN HEATER MODE - " + jsonData.intValue);
        }
    }

    if (path.startsWith(PREFIX_SETTINGS + String(KEY_TRIGGERS)))
    {
        parseTriggerJson(json, path);
        return;
    }

    if (json->get(jsonData, KEY_TRIGGERS, false))
    {
        printlnD("Parsing led triggers");
        printlnV(jsonData.type);

        FirebaseJson j;
        if (jsonData.getJSON(j))
        {
            parseTriggersJson(&j);
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

String Heater::getSettingKey() { return SETTINGS_HEATER_KEY; }

void Heater::parseValue(String key, String value)
{

    debugA("--Heater parse value, key = %s, value = %s\n", key.c_str(), value.c_str());


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
        case THERMO:
            setMode(THERMO);
            break;
        default:
            printlnW("UNKNOWN HEATER MODE - " + value.toInt());
        }
    }

    if (key.startsWith(PREFIX_SETTINGS + String(KEY_TRIGGERS)))
    {
        parseTriggerValue(key, value);
    }

    if (key == String(PREFIX_STATE) + KEY_HEATER_GOAL)
    {
        printlnA("Saving goal");
        setGoal(value.toDouble());
    }

    if (key == String(PREFIX_STATE) + FIREBASE_HEATER_CONNECTED_KEY)
    {
        setConnected(value == "true", false);
    }
}

bool Heater::updateSensorData(FirebaseJson *json)
{
    if (isConnected())
    {
        json->set(KEY_SENSOR_DATA_HEATER_POWER, _currentPower);
        json->set(KEY_SENSOR_DATA_HEATER_TEMP_GOAL, _settings.tempGoal);
        json->set("heater/ki", HEATER_KI);
        json->set("heater/kp", HEATER_KP);
        json->set("heater/kd", HEATER_KD);
        return true;
    }
    else
    {
        return false;
    }
}

bool Heater::getPayloadFromJson(FirebaseJson *json, double &payload)
{
    printlnA("getPayloadFromJson");
    FirebaseJsonData data;
    if (json->get(data, "/goal", false))
    {
        printlnA("got goal");
        payload = data.doubleValue;
        return true;
    }
    else
    {
        printlnW("No heater goal in JSON");
        return false;
    }
}

bool Heater::getPayloadFromValue(String key, String value, double &payload)
{
    printlnA("getPayloadFromJson");
    if (key == "goal")
    {
        printlnA("got goal");
        payload = value.toDouble();
        return true;
    }
    return false;
}

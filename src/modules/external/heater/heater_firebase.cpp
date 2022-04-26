#include "heater.h"
#include "QuickPID.h"
// JSON ;
#define KEY_SENSOR_DATA_HEATER_POWER "heater/power"
#define KEY_SENSOR_DATA_HEATER_TEMP_GOAL "heater/goal"

// Stream
#define KEY_HEATER_MODE "/heater/mode"
#define KEY_HEATER_TUNE_MODE "/heater/tMode"
#define KEY_TRIGGERS "/heater/triggers"
#define KEY_HEATER_TUNE "/heater/tune"
#define KEY_HEATER_DIRECT_POWER "/heater/dPower"

void Heater::parseJson(FirebaseJson *json, String path)
{
    printlnA("Heater - parseJson");
    FirebaseJsonData jsonData;

    if (json->get(jsonData, KEY_HEATER_MODE, false))
    {
        switch (jsonData.intValue)
        {
        case Mode::PID:
            setMode(PID);
            break;
        case Mode::AUTO:
            setMode(AUTO);
            break;
        case Mode::THERMO:
            setMode(THERMO);
            break;
        case Mode::DIRECT:
            setMode(DIRECT);
            break;
        default:
            printlnW("UNKNOWN HEATER MODE - " + jsonData.intValue);
        }
    }

    if (json->get(jsonData, KEY_HEATER_TUNE, false))
    {
        printlnA("Heater tune callback = " + jsonData.boolValue ? "true" : "False");
        _pid->setStartTuning(jsonData.boolValue);
    }

    if (json->get(jsonData, KEY_HEATER_TUNE_MODE))
    {
        _pid->setTuneMode(jsonData.intValue);
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

    if (json->get(jsonData, KEY_HEATER_DIRECT_POWER, false))
    {
        printlnA("SAVING Direct Power");
        _settings.directPower = jsonData.floatValue;
    }

    /// PID values

    if (json->get(jsonData, KEY_HEATER_KI, false))
    {
        _pid->setKi(jsonData.floatValue);
    }

    if (json->get(jsonData, KEY_HEATER_KP, false))
    {
        _pid->setKp(jsonData.floatValue);
    }

    if (json->get(jsonData, KEY_HEATER_KD, false))
    {
        _pid->setKd(jsonData.floatValue);
    }

    if (json->get(jsonData, KEY_HEATER_PON, false))
    {
        _pid->setPOn(jsonData.floatValue);
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
        case DIRECT:
            setMode(DIRECT);
            break;
        default:
            printlnW("UNKNOWN HEATER MODE - " + value.toInt());
        }
    }

    if (key.startsWith(PREFIX_STATE + String(KEY_HEATER_TUNE)))
    {
        printlnA("Heater tune parse value = " + value);
        _pid->setStartTuning(value == "true");
    }

    /// PID values

    if (key.startsWith(PREFIX_STATE + String(KEY_HEATER_KP)))
    {
        printlnA("Heater kp parse value = " + value);
        _pid->setKp(value.toFloat());
    }

    if (key.startsWith(PREFIX_STATE + String(KEY_HEATER_KI)))
    {
        printlnA("Heater ki parse value = " + value);
        _pid->setKi(value.toFloat());
    }

    if (key.startsWith(PREFIX_STATE + String(KEY_HEATER_KD)))
    {
        printlnA("Heater kd parse value = " + value);
        _pid->setKd(value.toFloat());
    }

    if (key.startsWith(PREFIX_STATE + String(KEY_HEATER_PON)))
    {
        printlnA("Heater pon parse value = " + value);
        _pid->setPOn(value.toFloat());
    }

    if (key.startsWith(PREFIX_SETTINGS + String(KEY_HEATER_TUNE_MODE)))
    {
        _pid->setTuneMode(value.toInt());
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

    if (key == String(PREFIX_SETTINGS) + KEY_HEATER_DIRECT_POWER)
    {
        printlnA("Saving Direct Power");
        _settings.directPower = value.toFloat();
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
        json->set(KEY_SENSOR_DATA_HEATER_POWER, (double)_currentPower);
        json->set(KEY_SENSOR_DATA_HEATER_TEMP_GOAL, (double)_settings.tempGoal);
        json->set("heater/ki", _pid->getKi());
        json->set("heater/kp", _pid->getKp());
        json->set("heater/kd", _pid->getKd());
        json->set("heater/tMode", _pid->getTuneMode());
        json->set("heater/tune", _pid->isAutoTuneRunning());
        json->set("heater/dPower", _settings.directPower);
        json->set("heater/pidMode", _pid->getMode());
        json->set("heater/pOn", _pid->getPOn());
        return true;
    }
    else
    {
        return false;
    }
}

bool Heater::getPayloadFromJson(FirebaseJson *json, double &payload)
{
    printlnD("getPayloadFromJson");
    FirebaseJsonData data;
    if (json->get(data, "/goal", false))
    {
        printD("got goal: ");
        payload = data.doubleValue;
        printlnD(payload) return true;
    }
    else
    {
        printlnW("No heater goal in JSON");
        return false;
    }
}

bool Heater::getPayloadFromValue(String key, String value, double &payload)
{
    printlnD("getPayloadFromJson");
    if (key == "goal")
    {
        printlnD("got goal: ");
        payload = value.toDouble();
        printD(payload);
        return true;
    }
    return false;
}

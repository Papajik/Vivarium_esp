#include "humidifier.h"

#define KEY_SENSOR_DATA_HUMIDIFIER "hum/isOn"

#define KEY_GOAL_HUM "/hum/goalHum"

void Humidifier::parseJson(FirebaseJson *data, String path)
{

    printlnA("Humidifier - parseJson");

    printlnV("Path: " + path);
    FirebaseJsonData jsonData;

    if (data->get(jsonData, KEY_GOAL_HUM, false))
    {
        setGoalHum(jsonData.floatValue);
    }

    if (data->get(jsonData, FIREBASE_HUM_CONNECTED_KEY, false))
    {
        setConnected(jsonData.boolValue, false);
    }
}

String Humidifier::getSettingKey() { return SETTINGS_HUMIDIFIER_KEY; }

void Humidifier::parseValue(String key, String value)
{
    printlnV("HUM parse value");
    printV("key = ");
    printlnV(key);
    printV("value = ");
    printlnV(value);

    if (key == String(PREFIX_SETTINGS) + KEY_GOAL_HUM)
    {

        setGoalHum(value.toFloat());
    }

    if (key == String(PREFIX_STATE) + FIREBASE_HUM_CONNECTED_KEY)
    {
        setConnected(value == "true", false);
    }
}

bool Humidifier::updateSensorData(FirebaseJson *json)
{
    if (isConnected())
    {
        json->set(KEY_SENSOR_DATA_HUMIDIFIER, _isOn);
        return true;
    }
    else
    {
        return false;
    }
}
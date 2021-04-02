#include "water_pump.h"

#define FIREBASE_PUMP_CONNECTED_KEY "/pump/connected"
#define KEY_GOAL_LEVEL "/pump/levelGoal"
#define KEY_SENSOR_DATA_PUMP_ON "pump/isOn"

void WaterPump::parseJson(FirebaseJson *data, String path)
{
    printlnA("Humidifier - parseJson");

    printlnV("Path: " + path);
    FirebaseJsonData jsonData;

    if (data->get(jsonData, KEY_GOAL_LEVEL, false))
    {
        setGoal(jsonData.intValue);
    }

    if (data->get(jsonData, FIREBASE_PUMP_CONNECTED_KEY, false))
    {
        setConnected(jsonData.boolValue, false);
    }
}

String WaterPump::getSettingKey() {printlnA("PUMP - settings key"); return SETTINGS_PUMP_KEY; }

void WaterPump::parseValue(String key, String value)
{
    printlnV("WATER PUMP parse value");
    printV("key = ");
    printlnV(key);
    printV("value = ");
    printlnV(value);

    if (key == String(PREFIX_SETTINGS) + KEY_GOAL_LEVEL)
    {
        setGoal(value.toInt());
    }

    if (key == String(PREFIX_STATE) + FIREBASE_PUMP_CONNECTED_KEY)
    {
        setConnected(value == "true", false);
    }
}

void WaterPump::updateSensorData(FirebaseJson *json)
{
    if (isConnected())
    {
        printlnA("upading sensor data");
        json->set(KEY_SENSOR_DATA_PUMP_ON, isRunning());
    }
}
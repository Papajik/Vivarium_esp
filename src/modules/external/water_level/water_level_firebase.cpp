#include "water_level.h"

#define KEY_SENSOR_DATA_W_LEVEL "wl/level"

#define KEY_SENSOR_HEIGHT "/wl/sensorHeight"
#define KEY_MAX_LEVEL "/wl/maxL"
#define KEY_MIN_LEVEL "/wl/minL"

String WaterLevel::getSettingKey() {return SETTINGS_WL_KEY; }

void WaterLevel::updateSensorData(FirebaseJson *json)
{
    if (isConnected())
    {
        json->set(KEY_SENSOR_DATA_W_LEVEL, _waterLevel);
    }
}

void WaterLevel::parseJson(FirebaseJson *data, String path)
{
    printlnA("WaterLevel - parseJson");

    printlnV("Path: " + path);
    FirebaseJsonData jsonData;

    if (data->get(jsonData, KEY_SENSOR_HEIGHT, false))
    {
        setSensorHeight(jsonData.intValue);
    }

    if (data->get(jsonData, KEY_MAX_LEVEL, false))
    {
        setMaxLevel(jsonData.intValue);
    }

    if (data->get(jsonData, KEY_MIN_LEVEL, false))
    {
        setMinLevel(jsonData.intValue);
    }

    if (data->get(jsonData, FIREBASE_WL_CONNECTED_KEY, false))
    {
        setConnected(jsonData.boolValue, false);
    }
}

void WaterLevel::parseValue(String key, String value)
{
    printlnV("WL parse value");
    printV("key = ");
    printlnV(key);
    printV("value = ");
    printlnV(value);

    if (key == String(PREFIX_SETTINGS) + KEY_SENSOR_HEIGHT)
    {

        setSensorHeight(value.toInt());
    }

    if (key == String(PREFIX_SETTINGS) + KEY_MAX_LEVEL)
    {

        setMaxLevel(value.toInt());
    }

    if (key == String(PREFIX_SETTINGS) + KEY_MIN_LEVEL)
    {

        setMinLevel(value.toInt());
    }

    if (key == String(PREFIX_STATE) + FIREBASE_WL_CONNECTED_KEY)
    {
        setConnected(value == "true", false);
    }
}
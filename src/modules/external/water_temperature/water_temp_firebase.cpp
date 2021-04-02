#include "water_temp.h"

#include <Firebase_ESP_Client.h>
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#define KEY_MAX_TEMP "/waterTemp/maxTemp"
#define KEY_MIN_TEMP "/waterTemp/minTemp"
#define KEY_CUR_TEMP "waterTemp/temp"

void WaterTempModule::parseJson(FirebaseJson *data, String path)
{
    printlnA("Water TEMP - parsing settings");

    FirebaseJsonData jsonData;

    if (data->get(jsonData, KEY_MAX_TEMP, false))
    {
        setMaxTemperature(jsonData.floatValue);
    }

    if (data->get(jsonData, KEY_MIN_TEMP, false))
    {
        setMinTemperature(jsonData.floatValue);
    }
    if (_settingsChanged)
    {
        saveSettings();
    }

    if (data->get(jsonData, FIREBASE_WT_CONNECTED_KEY, false))
    {
        setConnected(jsonData.boolValue, false);
    }
}
String WaterTempModule::getSettingKey()
{
    printlnA("WT - settings key");
    return SETTINGS_WATER_TEMP_KEY;
}
void WaterTempModule::parseValue(String key, String value)
{
    printlnV("Water temp parse value");
    printD("key = ");
    printlnD(key);
    printD("value = ");
    printlnD(value);

    if (key == String(PREFIX_SETTINGS) + KEY_MIN_TEMP)
    {
        setMinTemperature(value.toFloat());
    }

    if (key == String(PREFIX_SETTINGS) + KEY_MAX_TEMP)
    {
        setMaxTemperature(value.toFloat());
    }

    if (key == String(PREFIX_STATE) + FIREBASE_WT_CONNECTED_KEY)
    {
        setConnected(value == "true", false);
    }

    if (_settingsChanged)
    {
        saveSettings();
    }
}
void WaterTempModule::updateSensorData(FirebaseJson *json)
{
    // printlnV("update temp data");
    if (isConnected() && _currentTemp != WATER_TEMP_INVALID_VALUE)
    {
        json->set(KEY_CUR_TEMP, _currentTemp);
    }
}

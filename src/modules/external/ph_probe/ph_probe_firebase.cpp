#include "ph_probe.h"
#include <Firebase_ESP_Client.h>
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#define KEY_CONTINUOUS "/ph/continuous"
#define KEY_CONTINUOUS_DELAY "/ph/continuousDelay"
#define KEY_MAX_PH "/ph/maxPh"
#define KEY_MIN_PH "/ph/minPh"

/// SensorData
#define KEY_CURRENT_PH "ph/currentPh"

void PhModule::parseJson(FirebaseJson *data, String path)
{
    printlnA("PH - parseJson");
    FirebaseJsonData jsonData;
    if (data->get(jsonData, KEY_CONTINUOUS, false))
    {
        setContinuous(jsonData.boolValue);
        printA("Continuous: ");
        printlnA(_settings.continuous);
    }

    if (data->get(jsonData, KEY_CONTINUOUS_DELAY, false))
    {
        setContinuousDelay(jsonData.intValue);
    }

    if (data->get(jsonData, KEY_MAX_PH, false))
    {
        printA("setMaxPh: ");
        printlnA(_settings.max_ph);
        setMaxPh(jsonData.floatValue);
    }

    if (data->get(jsonData, KEY_MIN_PH, false))
    {
        setMinPh(jsonData.floatValue);
    }

    if (data->get(jsonData, FIREBASE_PH_CONNECTED_KEY, false))
    {
        printA("connected: ");
        printlnA(jsonData.boolValue);
        _connected = jsonData.boolValue;
        // Only start scanning on continuous mode
        if (_settings.continuous)
        {
            setConnected(false, false);
        }
    }
    if (settingsChanged)
    {
        printlnA("PH parseJson - settings changed");
        saveSettings();
    }
}

String PhModule::getSettingKey()
{
    return SETTINGS_PH_KEY;
}

void PhModule::parseValue(String key, String value)
{
    printlnA("PH parse value");
    printA("key = ");
    printlnA(key);
    printA("value = ");
    printlnA(value);

    if (key == String(PREFIX_SETTINGS) + KEY_CONTINUOUS)
    {
        setContinuous(value == "true");
    }

    if (key == String(PREFIX_SETTINGS) + KEY_CONTINUOUS_DELAY)
    {
        setContinuousDelay(value.toInt());
    }

    if (key == String(PREFIX_SETTINGS) + KEY_MAX_PH)
    {
        setMaxPh(value.toFloat());
    }

    if (key == String(PREFIX_SETTINGS) + KEY_MIN_PH)
    {
        setMinPh(value.toFloat());
    }

    if (key == String(PREFIX_STATE) + FIREBASE_PH_CONNECTED_KEY)
    {
        setConnected(value == "true", false);
    }

    if (settingsChanged)
    {
        printlnA("PH firebase parse value - settings changed");
        saveSettings();
    }
}

void PhModule::updateSensorData(FirebaseJson *json)
{
    if (_lastPhValue != PH_INVALID_VALUE)
    {
        json->set(KEY_CURRENT_PH, _lastPhValue);
        _lastPhValue = PH_INVALID_VALUE;
    }
}
#include "dht.h"

#define KEY_SENSOR_DATA_DHT_TEMP "dht/temp"
#define KEY_SENSOR_DATA_DHT_HUM "dht/hum"

#define KEY_MAX_HUM "/dht/maxHum"
#define KEY_MIN_HUM "/dht/minHum"
#define KEY_MAX_TEMP "/dht/maxTemp"
#define KEY_MIN_TEMP "/dht/minTemp"

void DhtModule::parseJson(FirebaseJson *data, String path)
{
    printlnA("DHT - parseJson");

    printlnV("Path: " + path);
    FirebaseJsonData jsonData;

    if (data->get(jsonData, KEY_MAX_HUM, false))
    {
        setMaxHum(jsonData.floatValue);
    }

    if (data->get(jsonData, KEY_MIN_HUM, false))
    {
        setMinHum(jsonData.floatValue);
    }

    if (data->get(jsonData, KEY_MAX_TEMP, false))
    {
        setMaxTemp(jsonData.floatValue);
    }

    if (data->get(jsonData, KEY_MIN_TEMP, false))
    {
        setMinTemp(jsonData.floatValue);
    }

    if (data->get(jsonData, FIREBASE_DHT_CONNECTED_KEY, false))
    {
        setConnected(jsonData.boolValue, false);
    }
}

String DhtModule::getSettingKey()
{
    printlnA("DHT - settings key");
    return SETTINGS_DHT_KEY;
}
void DhtModule::parseValue(String key, String value)
{
    printlnV("DHT parse value");
    printV("key = ");
    printlnV(key);
    printV("value = ");
    printlnV(value);

    if (key == String(PREFIX_SETTINGS) + KEY_MAX_HUM)
    {

        setMaxHum(value.toFloat());
    }

    if (key == String(PREFIX_SETTINGS) + KEY_MIN_HUM)
    {

        setMinHum(value.toFloat());
    }

    if (key == String(PREFIX_SETTINGS) + KEY_MAX_TEMP)
    {

        setMaxTemp(value.toFloat());
    }

    if (key == String(PREFIX_SETTINGS) + KEY_MIN_TEMP)
    {

        setMinTemp(value.toFloat());
    }

    if (key == String(PREFIX_STATE) + FIREBASE_DHT_CONNECTED_KEY)
    {
        setConnected(value == "true", false);
    }
}
void DhtModule::updateSensorData(FirebaseJson *json)
{
    if (isConnected())
    {
        if (_temp != DHT_INVALID_VALUE)
        {
            json->set(KEY_SENSOR_DATA_DHT_TEMP, _temp);
        }

        if (_humidity != DHT_INVALID_VALUE)
        {
            json->set(KEY_SENSOR_DATA_DHT_HUM, _humidity);
        }
    }
}
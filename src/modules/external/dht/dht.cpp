#include "dht.h"

#include <DHT.h>
#include "state_values.h"

#define CONNECTED_KEY "dht/c"
#define FIREBASE_STATE_TEMP "/sensorData/dht/temp"
#define FIREBASE_STATE_HUM "/sensorData/dht/hum"

#define FCM_KEY "DHT"

DhtModule::DhtModule(int pin) : IModule(CONNECTED_KEY)
{
    dht = new DHT(pin, DHT11);
    dht->begin();

    loadSettings();
}

void DhtModule::readDht()
{
    _lastRead = millis();
    float h = dht->readHumidity();
    float t = dht->readTemperature();
    printlnA("DHT Reading");
    printA("Temperature = ");
    printlnA(t);
    printA("Humidity  = ");
    printlnA(h);

    if (!isnan(h))
    {
        setHumidity(h);
    }
    else
    {
        setHumidity(DHT_INVALID_VALUE);
    }

    if (!isnan(t))
    {
        setTemp(t);
    }
    else
    {
        setTemp(DHT_INVALID_VALUE);
    }
}

void DhtModule::setHumidity(float h)
{
    if (h != _humidity)
    {

        _humidity = h;
        firebaseService.uploadCustomData("devices/", FIREBASE_STATE_HUM, _humidity);
        stateStorage.setValue(STATE_DHT_HUMIDITY, _humidity);
        if (isBluetoothRunning())
        {
            _humidityCharacteristic->setValue(_humidity);
            _humidityCharacteristic->notify();
        }
    }
}
void DhtModule::setTemp(float t)
{
    if (t != _temp)
    {
        _temp = t;
        firebaseService.uploadCustomData("devices/", FIREBASE_STATE_TEMP, _temp);
        stateStorage.setValue(STATE_DHT_TEMPERATURE, _temp);
        if (isBluetoothRunning())
        {
            _tempCharacteristic->setValue(_temp);
            _tempCharacteristic->notify();
        }
    }
}

float DhtModule::getTemp() { return _temp; }
float DhtModule::getHumidity() { return _humidity; }

void DhtModule::onLoop()
{
    checkConnectionChange();
    if (isConnected() && millis() - _lastRead > DHT_READ_DELAY)
    {
        readDht();
        checkBounds();
    }

    if (_settingsChanged)
    {
        saveSettings();
    }
}
void DhtModule::saveSettings()
{
    printlnV("DHT - Saving settings");
    memoryProvider.saveFloat(SETTINGS_DHT_MIN_TEMP, _minTemp);
    memoryProvider.saveFloat(SETTINGS_DHT_MAX_TEMP, _maxTemp);
    memoryProvider.saveFloat(SETTINGS_DHT_MIN_HUM, _minHum);
    memoryProvider.saveFloat(SETTINGS_DHT_MAX_HUM, _maxHum);

    _settingsChanged = false;
}

bool DhtModule::loadSettings()
{
    _minTemp = memoryProvider.loadFloat(SETTINGS_DHT_MIN_TEMP, DHT_INVALID_VALUE);
    _maxTemp = memoryProvider.loadFloat(SETTINGS_DHT_MAX_TEMP, DHT_INVALID_VALUE);
    _minHum = memoryProvider.loadFloat(SETTINGS_DHT_MIN_HUM, DHT_INVALID_VALUE);
    _maxHum = memoryProvider.loadFloat(SETTINGS_DHT_MAX_HUM, DHT_INVALID_VALUE);
    return true;
}
void DhtModule::onConnectionChange()
{
    if (_sourceIsButton)
    {
        firebaseService.uploadState(FIREBASE_DHT_CONNECTED_KEY, isConnected());
        _sourceIsButton = false;
    }

    stateStorage.setValue(STATE_DHT_CONNECTED, isConnected());

    sendConnectionChangeNotification(FCM_KEY, isConnected());

    if (isBluetoothRunning())
    {
        std::string s = isConnected() ? "true" : "false";
        _connectedCharacteristic->setValue(s);
        _connectedCharacteristic->notify();
    }
}

void DhtModule::setMaxHum(float h)
{
    if (h != _maxHum)
    {
        _settingsChanged = true;
        _maxHum = h;
    }
}
void DhtModule::setMinHum(float h)
{
    if (h != _minHum)
    {
        _settingsChanged = true;
        _minHum = h;
    }
}

void DhtModule::setMaxTemp(float h)
{
    if (h != _maxTemp)
    {
        _settingsChanged = true;
        _maxTemp = h;
    }
}
void DhtModule::setMinTemp(float h)
{
    if (h != _minTemp)
    {
        _settingsChanged = true;
        _minTemp = h;
    }
}

void DhtModule::checkBounds()
{
    if (_temp != DHT_INVALID_VALUE)
    {
        // upper
        if (_temp > _maxTemp)
        {
            messagingService.sendFCM(FCM_KEY, "Temperature is over maximum alowed value", FCM_TYPE::CROSS_LIMIT, SETTINGS_DHT_KEY);
        }
        if (_temp < _minTemp)
        {
            messagingService.sendFCM(FCM_KEY, "Temperature is below minimum alowed value", FCM_TYPE::CROSS_LIMIT, SETTINGS_DHT_KEY);
        }
    }

    if (_humidity != DHT_INVALID_VALUE)
    {
        // upper
        if (_humidity > _maxHum)
        {
            messagingService.sendFCM(FCM_KEY, "Humidity is  over maximum alowed value", FCM_TYPE::CROSS_LIMIT, SETTINGS_DHT_KEY);
        }
        if (_humidity < _minHum)
        {
            messagingService.sendFCM(FCM_KEY, "Humidity is below minimum alowed", FCM_TYPE::CROSS_LIMIT, SETTINGS_DHT_KEY);
        }
    }
}

float DhtModule::getMaxHum() { return _maxHum; }
float DhtModule::getMinHum() { return _minHum; }

float DhtModule::getMaxTemp() { return _maxTemp; }
float DhtModule::getMinTemp() { return _minTemp; }
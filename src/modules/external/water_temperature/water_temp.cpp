#include "water_temp.h"
//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#include "millisDelay.h"

#include <OneWire.h>
#include <DallasTemperature.h>

#include "state_values.h"

#define CONNECTED_KEY "temp/c"
#define FIREBASE_STATE_TEMP "/sensorData/waterTemp/temp"

#define WATER_TEMP_READ_DELAY 500

WaterTempModule::WaterTempModule(int position, int pin) : IModule(CONNECTED_KEY, position)
{
    printlnA("Water temp module created");
    _settings = {10, 20};

    _oneWire = new OneWire(pin);
    _dallas = new DallasTemperature(_oneWire);
    _dallas->begin();
    _delay = new millisDelay();
}

WaterTempModule::~WaterTempModule()
{
    delete _delay;
    delete _oneWire;
    delete _dallas;
}

void WaterTempModule::onConnectionChange()
{
    if (isConnected())
    {
        startReadings();
        readTemperature();
        stateStorage.setValue(STATE_WATER_TEMP_CONNECTED, true);
    }
    else
    {
        stateStorage.setValue(STATE_WATER_TEMP_CONNECTED, false);
        stopReading();
    }
    if (_sourceIsButton)
    {
        firebaseService->uploadState(FIREBASE_WT_CONNECTED_KEY, isConnected());
        _sourceIsButton = false;
    }

    if (isBluetoothRunning())
    {
        std::string s = isConnected() ? "true" : "false";
        _connectedCharacteristic->setValue(s);
        _connectedCharacteristic->notify();
    }

    sendConnectionChangeNotification("Water Temp", isConnected());
}
void WaterTempModule::onLoop()
{
    if (_settingsChanged)
    {
        saveSettings();
    }

    checkConnectionChange();

    if (isConnected())
    {
        if (_delay->justFinished())
        {
            readTemperature();
            _delay->restart();
        }
    }
}

void WaterTempModule::setMinTemperature(float temp)
{
    if (_settings.min_temp != temp)
    {
        _settings.min_temp = temp;
        _settingsChanged = true;
    }
}
void WaterTempModule::setMaxTemperature(float temp)
{
    if (_settings.max_temp != temp)
    {
        _settings.max_temp = temp;
        _settingsChanged = true;
    }
}

float WaterTempModule::getMinTemperature()
{
    return _settings.min_temp;
}
float WaterTempModule::getMaxTemperature()
{
    return _settings.max_temp;
}

float WaterTempModule::getCurrentTemperature()
{
    return _currentTemp;
}

void WaterTempModule::startReadings()
{
    _delay->start(WATER_TEMP_READ_DELAY);
}
void WaterTempModule::stopReading()
{
    _delay->stop();
    stateStorage.setValue(STATE_WATER_TEMPERATURE, (float)WATER_TEMP_INVALID_VALUE);
    _currentTemp = WATER_TEMP_INVALID_VALUE;
}

void WaterTempModule::saveSettings()
{
    _memoryProvider->saveStruct(SETTINGS_WATER_TEMP_KEY, &_settings, sizeof(WaterTempSettings));
    _settingsChanged = false;
}
bool WaterTempModule::loadSettings()
{
    return _memoryProvider->loadStruct(SETTINGS_WATER_TEMP_KEY, &_settings, sizeof(WaterTempSettings));
}

void WaterTempModule::readTemperature()
{
    printlnV("\n");
    printlnV("READ TEMPERATURE");
    _dallas->requestTemperatures();
    float old = _currentTemp;
    _currentTemp = _dallas->getTempCByIndex(0);
    stateStorage.setValue(STATE_WATER_TEMPERATURE, _currentTemp);
    if (old != _currentTemp)
    {

        printlnD("Uploading new temperature");
        firebaseService->uploadCustomData("devices/", FIREBASE_STATE_TEMP, _currentTemp);
        if (isBluetoothRunning())
        {
            _currentTempCharacteristic->setValue(_currentTemp);
            _currentTempCharacteristic->notify();
        }
        checkBoundaries();
    }

    printlnV("Current temp = " + String(_currentTemp));
}

void WaterTempModule::checkBoundaries()
{
    if (_currentTemp != WATER_TEMP_INVALID_VALUE)
    {
        if (_currentTemp > _settings.max_temp)
        {
            messagingService->sendFCM(SETTINGS_WATER_TEMP_KEY, "Temperature is over maximum alowed value", FCM_TYPE::CROSS_LIMIT, SETTINGS_WATER_TEMP_KEY);
        }

        if (_currentTemp < _settings.min_temp)
        {
            messagingService->sendFCM(SETTINGS_WATER_TEMP_KEY, "Temperature is below maximum alowed value", FCM_TYPE::CROSS_LIMIT, SETTINGS_WATER_TEMP_KEY);
        }
    }
}
#include "water_temp.h"
//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#include "millisDelay.h"

#include <OneWire.h>
#include "../../../lib/dallasTemperature/DallasTemperature.h"

#include "state_values.h"

#define CONNECTED_KEY "temp/c"
#define FIREBASE_STATE_TEMP "/sensorData/waterTemp/temp"

#define WATER_TEMP_READ_DELAY 2000
#define WATER_TEMP_MAX_INVALID_READINGS_IN_ROW 3

#define OK_MESSAGE_DELAY

WaterTempModule::WaterTempModule(int position, MemoryProvider *m, int pin)
    : IModule(CONNECTED_KEY, position, m),
      _oneWire(new OneWire(pin)),
      _dallas(new DallasTemperature(_oneWire)),
      _delay(new millisDelay())
{
    printlnA("Water temp module created");
    if (!loadSettings())
    {
        _settings = {10, 20};
    }
    setInnerState(_dallas);

    _dallas->begin();
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

    setStep(0);
    if (_settingsChanged)
    {
        saveSettings();
    }
    setStep(1);

    checkConnectionChange();

    setStep(2);
    if (isConnected())
    {
        if (_delay->justFinished())
        {
            setStep(3);
            setMillis();
            readTemperature();
            setStep(10);
            _delay->repeat();
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
    setStep(4);
    _dallas->requestTemperatures();
    setStep(5);
    float temp = _dallas->getTempCByIndex(0);
    setStep(6);
    /// Skip first x invalid temperatures
    if (temp == WATER_TEMP_INVALID_VALUE && _invalidReadingInRow < WATER_TEMP_MAX_INVALID_READINGS_IN_ROW)
    {
        _invalidReadingInRow++;
        return;
    }
    setStep(7);

    if (temp != WATER_TEMP_INVALID_VALUE)
    {
        _invalidReadingInRow = 0;
    }
    setStep(8);

    stateStorage.setValue(STATE_WATER_TEMPERATURE, temp);
    setStep(9);
    if (temp != _currentTemp)
    {
        _currentTemp = temp;
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
    if (_currentTemp != WATER_TEMP_INVALID_VALUE && messagingService != nullptr)
    {
        if (_currentTemp > _settings.max_temp)
        {
            char buffer[80];
            sprintf(buffer, "Water temperature (%.2f °C) is over maximum alowed value (%.2f °C)", _currentTemp, _settings.max_temp);
            messagingService->sendFCM(SETTINGS_WATER_TEMP_KEY, String(buffer), FCM_TYPE::CROSS_LIMIT, String(SETTINGS_WATER_TEMP_KEY) + "b");
            _lastMessage = 1;
            _messageCount = 0;
            return;
        }

        if (_currentTemp < _settings.min_temp)
        {
            char buffer[80];
            sprintf(buffer, "Water temperature (%.2f °C) is below minimum alowed value (%.2f °C)", _currentTemp, _settings.min_temp);
            messagingService->sendFCM(SETTINGS_WATER_TEMP_KEY, String(buffer), FCM_TYPE::CROSS_LIMIT, String(SETTINGS_WATER_TEMP_KEY) + "b");
            _lastMessage = -1;
            _messageCount = 0;
            return;
        }

        /// If boundaries are ok:
        if (_lastMessage != 0 || (_lastMessage == 0 && _messageCount < 10))
        {
            char buffer[50];
            sprintf(buffer, "Water temperature (%.2f °C) is OK", _currentTemp);
            messagingService->sendFCM(SETTINGS_WATER_TEMP_KEY, String(buffer), FCM_TYPE::CROSS_LIMIT, String(SETTINGS_WATER_TEMP_KEY) + "b");
            _lastMessage = 0;
            _messageCount++;
            _lastMessageTime = millis();
        }
    }
}

std::vector<String> WaterTempModule::getText()
{
    if (!_connected)
    {
        return {"Water Temp", "Disconnected"};
    }
    else
    {
        return {"Water Temp: " + String(_currentTemp, 1), "LL: " + String(_settings.min_temp, 1) + " HL: " + String(_settings.max_temp, 1)};
    }
}
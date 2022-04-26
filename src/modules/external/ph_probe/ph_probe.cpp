#include "ph_probe.h"

//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//Delays
#include <millisDelay.h>

// string to float
#include <stdlib.h>

#include "../../../state/state.h"
#include "../../../utils/compare.h"

#include "state_values.h"
#define KEYWORD_WATER_PH "waterPh"
#define KEYWORD_WATER_PH_MAX "waterMaxPh"
#define KEYWORD_WATER_PH_MIN "waterMinPh"

#define CONNECTED_KEY "ph/c"
#define FIREBASE_CD_PH "/sensorData/ph/ph"
#define FCM_KEY "pH Probe"

// Firmware & calibration settings

#define PH_CALIBRATION 20.24
#define PH_VALUES_CUT 2
#define PH_VOLTAGE_CHANGE -8.536
#define PH_VOLTAGE 3.3
#define PH_PRECISION 4096

#define PH_TIMER_ONCE_DELAY 15
#define PH_TIMER_CONTINUOUS_DELAY 20
#define PH_DEFAULT_CONTINUOUS_DELAY 60000 // 1 minute

PhModule::PhModule(int position, MemoryProvider *m, int pin) : IModule(CONNECTED_KEY, position, m), _pin(pin)
{

    printlnA("PhModule created");

    if (!loadSettings())
    {
        _settings = {false, PH_DEFAULT_CONTINUOUS_DELAY, 3, 4};
    }

    pinMode(pin, INPUT);
    _delay = new millisDelay();
}

float PhModule::_readPh()
{
    printlnD("READING PH");
    for (int i = 0; i < PH_READING_COUNT; i++)
    {
        _phReadBuffer[i] = analogRead(_pin);
        delay(30);
    }

    qsort(_phReadBuffer, PH_READING_COUNT, sizeof(float), compareFloat);

    float phAvgValue = 0;
    for (int i = PH_VALUES_CUT; i < PH_READING_COUNT - PH_VALUES_CUT; i++)
    {
        phAvgValue += _phReadBuffer[i];
    }

    phAvgValue /= PH_READING_COUNT - (2 * PH_VALUES_CUT);

    printA("pH Value = ");
    printlnA(phAvgValue);

    float Po = (float)phAvgValue * PH_VOLTAGE / PH_PRECISION;

    float phValue = PH_VOLTAGE_CHANGE * Po * PH_CALIBRATION;
    if (phValue < 0 || phValue > 14)
    {
        phValue = PH_INVALID_VALUE;
    }

    return phValue;
}

void PhModule::_setPhValue(float value)
{
    if (_lastPhValue != value)
    {
        _lastPhValue = value;
        if (isBluetoothRunning())
        {
            characteristicWaterPh->setValue(value);
            characteristicWaterPh->notify();
        }
        firebaseService->uploadCustomData("devices/", FIREBASE_CD_PH, value);

        checkBoundaries();
    }
}

void PhModule::checkContinuousScan()
{

    if (_settings.continuous)
    {
        if (_delay->justFinished())
        {
            _setPhValue(_readPh());
            _delay->restart();
        }
    }
}

void PhModule::startScan()
{
    printlnA("Scan start");

    if (_settings.continuous)
    {
        if (_delay->isRunning())
        {
            _delay->stop();
        }
        _delay->start(_settings.continuous_delay);
    }
    _setPhValue(_readPh());
    if (!_settings.continuous)
    {
        printText({"pH Probe", "pH: " + String(_lastPhValue)}, 5000);
    }
}

void PhModule::stopScan()
{
    printlnA("Scan stop");
    if (_delay->isRunning())
    {
        _delay->stop();
    }
}

void PhModule::setContinuous(bool m)
{
    if (m != _settings.continuous)
    {
        _settings.continuous = m;
        setSettingsChanged(true);
    }
}

bool PhModule::isContinuous()
{
    return _settings.continuous;
}

void PhModule::onConnectionChange()
{

    printlnA("PH probe onConnectionChange");
    if (isConnected())
    {
        startScan();
    }
    else
    {
        stopScan();
    }

    if (_sourceIsButton)
    {
        firebaseService->uploadState(FIREBASE_PH_CONNECTED_KEY, isConnected());
        _sourceIsButton = false;
    }

    sendConnectionChangeNotification("pH Probe", isConnected());

    if (isBluetoothRunning())
    {
        std::string s = isConnected() ? "true" : "false";
        _connectedCharacteristic->setValue(s);
        _connectedCharacteristic->notify();
    }
}

void PhModule::saveSettings()
{
    printlnA("PH - save settings");

    _memoryProvider->saveStruct(SETTINGS_PH_KEY, &_settings, sizeof(PhModuleSettings));
    setSettingsChanged(false);
}

bool PhModule::loadSettings()
{
    printlnA("Loading settings");
    return _memoryProvider->loadStruct(SETTINGS_PH_KEY, &_settings, sizeof(PhModuleSettings));
}

void PhModule::onBLEConnect()
{
}

void PhModule::onLoop()
{

    if (settingsChanged)
    {
        printlnA("PH proble - on loop - settings changed");
        saveSettings();
    }

    // Check connection
    // update connection on change
    checkConnectionChange();

    if (_connected)
    {
        checkContinuousScan();
    }
}

void PhModule::setMaxPh(float ph)
{

    if (_settings.max_ph != ph)
    {
        _settings.max_ph = ph;
        setSettingsChanged(true);
    }
}

void PhModule::setMinPh(float ph)
{
    if (_settings.min_ph != ph)
    {
        _settings.min_ph = ph;
        setSettingsChanged(true);
    }
}

float PhModule::getMaxPh()
{
    return _settings.max_ph;
}

float PhModule::getMinPh()
{
    return _settings.min_ph;
}

void PhModule::setContinuousDelay(int n)
{
    if (_settings.continuous_delay != n * 1000)
    {
        _settings.continuous_delay = n * 1000;
        if (_delay->isRunning())
        {
            startScan();
        }
        setSettingsChanged(true);
    }
}

int PhModule::getContinuousDelay()
{
    return _settings.continuous_delay / 1000;
}

float PhModule::getPhValue()
{
    return _lastPhValue;
}

void PhModule::checkBoundaries()
{
    if (messagingService == nullptr)
        return;
    if (_lastPhValue > _settings.max_ph)
    {
        char buffer[50];
        sprintf(buffer, "pH (%.2f) is over maximum alowed value (%.2f)", _lastPhValue, _settings.max_ph);
        messagingService->sendFCM(FCM_KEY, String(buffer), FCM_TYPE::CROSS_LIMIT, String(FCM_KEY) + "h");
    }

    if (_lastPhValue < _settings.min_ph)
    {
        char buffer[50];
        sprintf(buffer, "pH (%.2f) is below minimum alowed value (%.2f)", _lastPhValue, _settings.min_ph);
        messagingService->sendFCM(FCM_KEY, String(buffer), FCM_TYPE::CROSS_LIMIT, String(FCM_KEY) + "l");
    }
}

void PhModule::setSettingsChanged(bool b)
{
    printA("PH probe - settings changed to ");
    printlnA(b ? "True" : "false");
}

std::vector<String> PhModule::getText()
{
    if (!_connected)
    {
        return {"pH Probe", "Disconnected"};
    }
    else
    {
        return {"pH Probe", "pH: " + String(_lastPhValue)};
    }
}
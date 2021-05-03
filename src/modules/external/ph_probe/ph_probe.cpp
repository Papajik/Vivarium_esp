#include "ph_probe.h"

//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//Delays
#include <millisDelay.h>

// string to float
#include <stdlib.h>

#include "../../../state/state.h"

#include "../../../wifi/wifiProvider.h"
#include "state_values.h"
#define KEYWORD_WATER_PH "waterPh"
#define KEYWORD_WATER_PH_MAX "waterMaxPh"
#define KEYWORD_WATER_PH_MIN "waterMinPh"

#define CONNECTED_KEY "ph/c"
#define FIREBASE_CD_PH "/sensorData/ph/ph"
#define FCM_KEY "pH Probe"

int _comparePh(const void *arg1, const void *arg2)
{
    int *a = (int *)arg1;
    int *b = (int *)arg2;
    if (*a < *b)
        return -1;
    if (*a > *b)
        return 1;
    return 0;
}

PhModule::PhModule() : IModule(CONNECTED_KEY)
{
    printlnA("PhModule created");
    if (!loadSettings())
    {
        _settings = {false, PH_DEFAULT_CONTINUOUS_DELAY, 3, 4};
    }

    pinMode(PH_PIN, INPUT);
    _delay = new millisDelay();
    //stateStorage.setCallback(STATE_WATER_PH, new ChangePhCallback(this), type_float);
}

float PhModule::_readPh()
{
    printlnD("READING PH");
    for (int i = 0; i < PH_READING_COUNT; i++)
    {
        _phReadBuffer[i] = analogRead(PH_PIN);
        delay(30);
    }

    qsort(_phReadBuffer, PH_READING_COUNT, sizeof(float), _comparePh);

    float phAvgValue = 0;
    for (int i = PH_VALUES_CUT; i < PH_READING_COUNT - PH_VALUES_CUT; i++)
    {
        phAvgValue += _phReadBuffer[i];
    }

    float Po = (float)phAvgValue * 5.0 / 1024 / (PH_READING_COUNT - 2 * PH_VALUES_CUT);
    float phValue = 7 - (2.5 - Po) * PH_VOLTAGE_CHANGE;
    if (phValue < 0 || phValue > 7)
    {
        phValue = PH_INVALID_VALUE;
    }

    _setPhValue(phValue);
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
        firebaseService.uploadCustomData("devices/", FIREBASE_CD_PH, value);

        checkBoundaries();
    }
}

void PhModule::checkContinuousScan()
{

    if (_settings.continuous)
    {
        if (_delay->justFinished())
        {
            _readPh();
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

    _readPh();
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
        firebaseService.uploadState(FIREBASE_PH_CONNECTED_KEY, isConnected());
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

    memoryProvider.saveStruct(SETTINGS_PH_KEY, &_settings, sizeof(PhModuleSettings));
    setSettingsChanged(false);
}

bool PhModule::loadSettings()
{
    printlnA("Loading settings");
    return memoryProvider.loadStruct(SETTINGS_PH_KEY, &_settings, sizeof(PhModuleSettings));
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
    //Check connection

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
    if (_lastPhValue > _settings.max_ph)
    {
        messagingService.sendFCM(FCM_KEY, "pH is over maximum alowed value", FCM_TYPE::CROSS_LIMIT, FCM_KEY);
    }

    if (_lastPhValue < _settings.min_ph)
    {
        messagingService.sendFCM(FCM_KEY, "pH is below maximum alowed value", FCM_TYPE::CROSS_LIMIT, FCM_KEY);
    }
}

void PhModule::setSettingsChanged(bool b)
{
    printA("PH probe - settings changed to ");
    printlnA(b ? "True" : "false");
}
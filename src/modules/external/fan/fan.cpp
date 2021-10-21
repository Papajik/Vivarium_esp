#include "fan.h"

#include "../../../state/state_values.h"

#include <HardwareSerial.h>
#include "state_values.h"
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#define CONNECTED_KEY "fan/c"
#define FIREBASE_FAN_SPEED "/fan/speed"

#define FAN_RESOLUTION 8
#define FAN_FREQUENCY 40
#define FAN_CHANNEL 0

FanController::FanController(int position, MemoryProvider *m, int pin) : IModule(CONNECTED_KEY, position, m)
{
    printlnA("Fan controller created");
    if (!loadSettings())
    {
        _settings = {20, 30};
    }
    ledcSetup(FAN_CHANNEL, FAN_FREQUENCY, FAN_RESOLUTION);
    ledcAttachPin(pin, FAN_CHANNEL);
}

void FanController::setStartAt(float f)
{
    if (_settings.setStartAt != f)
    {
        _settings.setStartAt = f;
        _settingsChanged = true;
    }
}

float FanController::getMaxAt()
{
    return _settings.setMaxAt;
}

float FanController::getStartAt()
{
    return _settings.setStartAt;
}

int FanController::getSpeed()
{
    return _currentSpeed;
}

void FanController::setMaxAt(float f)
{
    if (_settings.setMaxAt != f)
    {
        _settings.setMaxAt = f;
        _settingsChanged = true;
    }
}

void FanController::onLoop()
{
    if (_settingsChanged)
    {
        saveSettings();
    }
    checkConnectionChange();
    if (isConnected())
    {
        float temp;
        if (stateStorage.getValue(STATE_WATER_TEMPERATURE, &temp))
        {
            parseFanSpeed(temp);
        }
    }
    else
    {
        setSpeed(FAN_STOP_SPEED);
    }
}

void FanController::parseFanSpeed(float temp)
{
    if (temp < _settings.setStartAt)
    {
        setSpeed(FAN_STOP_SPEED);
        return;
    }
    if (temp > _settings.setMaxAt)
    {
        setSpeed(FAN_MAX_SPEED);
        return;
    }
    int speed = (FAN_MAX_SPEED - FAN_MIN_SPEED) / (_settings.setMaxAt - _settings.setStartAt) * (temp - _settings.setStartAt) + FAN_MIN_SPEED;

    setSpeed(speed);
}

float FanController::getPercantage(int speed)
{
    return speed / FAN_MAX_SPEED * 100;
}

void FanController::setSpeed(int speed)
{
    if (_currentSpeed != speed)
    {
        ledcWrite(FAN_CHANNEL, speed);
        _currentSpeed = speed;
        stateStorage.setValue(FAN_SPEED, (uint32_t)speed);
        if (isBluetoothRunning())
        {
            _currentSpeedCharacteristic->setValue(_currentSpeed);
            _currentSpeedCharacteristic->notify();
        }
        if (firebaseService != nullptr)
            firebaseService->uploadState(FIREBASE_FAN_SPEED, getPercantage(_currentSpeed));
    }
}

void FanController::saveSettings()
{
    _memoryProvider->saveStruct(SETTINGS_FAN_KEY, &_settings, sizeof(FanSettings));
    _settingsChanged = false;
}
bool FanController::loadSettings()
{
    return _memoryProvider->loadStruct(SETTINGS_FAN_KEY, &_settings, sizeof(FanSettings));
}

void FanController::onConnectionChange()
{
    if (_sourceIsButton)
    {
        firebaseService->uploadState(FIREBASE_FAN_CONNECTED_KEY, isConnected());
        _sourceIsButton = false;
    }

    if (isBluetoothRunning())
    {
        std::string s = isConnected() ? "true" : "false";
        _connectedCharacteristic->setValue(s);
        _connectedCharacteristic->notify();
    }

    sendConnectionChangeNotification("FAN", isConnected());
}

std::vector<String> FanController::getText()
{
    if (!_connected)
    {
        return {"Fan", "Disconnected"};
    }
    else
    {
        return {"Fan: " + String(_currentSpeed / 255 * 100) + " %", "S:" + String(_settings.setStartAt, 1) + "M: " + String(_settings.setMaxAt, 1)};
    }
}
#include "water_pump.h"
#include "../../../state/state_values.h"

#define CONNECTED_KEY "pump/c"
#define SETTINGS_PUMP_GOAL_KEY "pumpG"
#define FIREBASE_IS_ON_STATE "/pump/isOn"
#define FCM_KEY "Water Pump"

WaterPump::WaterPump(int position, int pin)
    : IModule(CONNECTED_KEY, position),
      _pin(pin)
{
    pinMode(_pin, OUTPUT);
}

void WaterPump::startPump()
{
    if (!_running)
    {
        printlnA("START PUMP");
        _running = true;
        digitalWrite(_pin, HIGH);

        firebaseService->uploadState(FIREBASE_IS_ON_STATE, true);

        if (isBluetoothRunning())
        {
            _pumpRunningCharacteristic->setValue("true");
            _pumpRunningCharacteristic->notify();
        }

        if (messagingService != nullptr)
        {
            messagingService->sendFCM(FCM_KEY, "Start", FCM_TYPE::TRIGGER, FCM_KEY);
        }
    }
}
void WaterPump::stopPump()
{
    if (_running)
    {
        printlnA("STOP PUMP");
        _running = false;
        digitalWrite(_pin, LOW);
        firebaseService->uploadState(FIREBASE_IS_ON_STATE, false);
        if (isBluetoothRunning())
        {
            _pumpRunningCharacteristic->setValue("false");
            _pumpRunningCharacteristic->notify();
        }

        if (messagingService != nullptr)
        {
            messagingService->sendFCM(FCM_KEY, "Stop", FCM_TYPE::TRIGGER, FCM_KEY);
        }
    }
}

void WaterPump::beforeShutdown()
{
    digitalWrite(_pin, LOW);
}

bool WaterPump::isRunning()
{
    return _running;
}

void WaterPump::onLoop()
{
    checkConnectionChange();
    if (_settingsChanged)
    {
        saveSettings();
    }

    if (isConnected())
    {
        // Fail safe
        bool wlConnected;
        stateStorage.getValue(STATE_WATER_LEVEL_CONNECTED, &wlConnected);
        if (!wlConnected)
        {
            stopPump();
            setConnected(false, true);
            return;
        }

        uint32_t level;
        if (stateStorage.getValue(STATE_WATER_LEVEL, &level))
        {
            if (level < _levelGoal)
            {
                startPump();
            }
            else
            {
                stopPump();
            }
        }
    }
}

void WaterPump::saveSettings()
{
    printlnA("Fan - save settings");
    _memoryProvider->saveInt(SETTINGS_PUMP_GOAL_KEY, _levelGoal);
    _settingsChanged = false;
}
bool WaterPump::loadSettings()
{
    _levelGoal = _memoryProvider->loadInt(SETTINGS_PUMP_GOAL_KEY, 0);
    return true;
}

void WaterPump::onConnectionChange()
{
    if (_sourceIsButton)
    {
        firebaseService->uploadState(FIREBASE_PUMP_CONNECTED_KEY, isConnected());
        _sourceIsButton = false;
    }

    if (isBluetoothRunning())
    {
        std::string s = isConnected() ? "true" : "false";
        _connectedCharacteristic->setValue(s);
        _connectedCharacteristic->notify();
    }

    if (!isConnected())
    {
        stopPump();
    }

    sendConnectionChangeNotification("Pump", isConnected());
}

void WaterPump::setGoal(int g)
{
    _levelGoal = g;
}
int WaterPump::getGoal()
{
    return _levelGoal;
}

std::vector<String> WaterPump::getText()
{
    if (!_connected)
    {
        return {"Water Pump", "Disconnected"};
    }
    else
    {
        return {
            "Water Pump" + _running ? "ON" : "OFF",
            "Goal: " + String(_levelGoal) + " cm"};
    }
}
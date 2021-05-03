#include "water_pump.h"
#include "../../../state/state_values.h"

#define CONNECTED_KEY "pump/c"
#define SETTINGS_PUMP_GOAL_KEY "pumpG"
#define FIREBASE_IS_ON_STATE "/pump/isOn"

#define WATER_PUMP_PIN (int)26


WaterPump::WaterPump() : IModule(CONNECTED_KEY)
{
}

void WaterPump::startPump()
{
    _running = true;
    digitalWrite(WATER_PUMP_PIN, HIGH);

    firebaseService.uploadState(FIREBASE_IS_ON_STATE, true);

    if (isBluetoothRunning())
    {
        _pumpRunningCharacteristic->setValue("true");
        _pumpRunningCharacteristic->notify();
    }
}
void WaterPump::stopPump()
{
    _running = false;
    digitalWrite(WATER_PUMP_PIN, LOW);
    firebaseService.uploadState(FIREBASE_IS_ON_STATE, false);
    if (isBluetoothRunning())
    {
        _pumpRunningCharacteristic->setValue("false");
        _pumpRunningCharacteristic->notify();
    }
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
            setConnected(false, true);
            return;
        }

        //
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
    memoryProvider.saveInt(SETTINGS_PUMP_GOAL_KEY, _levelGoal);
    _settingsChanged = false;
}
bool WaterPump::loadSettings()
{
    _levelGoal = memoryProvider.loadInt(SETTINGS_PUMP_GOAL_KEY, 0);
    return true;
}

void WaterPump::onConnectionChange()
{
    if (_sourceIsButton)
    {
        firebaseService.uploadState(FIREBASE_PUMP_CONNECTED_KEY, isConnected());
        _sourceIsButton = false;
    }

    if (isBluetoothRunning())
    {
         std::string s = isConnected()?"true":"false";
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

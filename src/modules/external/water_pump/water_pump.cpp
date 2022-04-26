#include "water_pump.h"
#include "../../../state/state_values.h"

#define CONNECTED_KEY "pump/c"
#define SETTINGS_PUMP_GOAL_KEY "pumpG"
#define FIREBASE_IS_ON_STATE "/pump/isOn"
#define FCM_KEY "Water Pump"

hw_timer_t *timer = nullptr;

void IRAM_ATTR stopPumpTimer()
{
    ets_printf("\nStop pump timer out\n"); // non blocking output with ability to be used inside ISR function
    waterPumpPtr->stopPumpFailSafe();
}

WaterPump *waterPumpPtr = nullptr;

WaterPump::WaterPump(int position, MemoryProvider *m, int pin)
    : IModule(CONNECTED_KEY, position, m),
      _pin(pin)
{
    loadSettings();
    pinMode(_pin, OUTPUT);
    timer = timerBegin(0, 80, true); // div 80 Mhz
    timerAttachInterrupt(timer, &stopPumpTimer, true);
    timerAlarmWrite(timer, 2000 * 1000, false); //2 seconds
    timerAlarmDisable(timer);
}

WaterPump::~WaterPump()
{
    timerStop(timer);
    timerAlarmDisable(timer);
}

void WaterPump::stopPumpFailSafe()
{
    digitalWrite(_pin, LOW);
    _failSafeTriggered = true;
}

void WaterPump::startPump()
{
    timerWrite(timer, 0);
    if (!_running)
    {
        timerAlarmEnable(timer);
        printlnV("START PUMP");
        _running = true;
        digitalWrite(_pin, HIGH);

        if (firebaseService != nullptr)
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
        timerAlarmDisable(timer);
        _running = false;
        digitalWrite(_pin, LOW);
        if (firebaseService != nullptr)
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
        bool wlConnected = false;
        stateStorage.getValue(STATE_WATER_LEVEL_CONNECTED, &wlConnected);
        if (!wlConnected)
        {
            stopPump();
            setConnected(false, true);
            checkConnectionChange();
            return;
        }

        // Fail safe #2
        if (isFailSafeTriggered())
        {
            Serial.println("Second fail save triggered - stopping pump");
            stopPump();
            _failSafeTriggered = false;
        }

        uint32_t level;
        if (stateStorage.getValue(STATE_WATER_LEVEL, &level))
        {
            if (level == WATER_LEVEL_INVALID_VALUE)
            {
                stopPump();
                return;
            }
            if (level < _levelGoal)
            {
                startPump();
            }
            else
            {
                stopPump();
            }
        }
        else
        {
            stopPump();
        }
    }
}

void WaterPump::saveSettings()
{
    printlnA("Fan - save settings");
    if (_memoryProvider != nullptr)
    {
        _memoryProvider->saveInt(SETTINGS_PUMP_GOAL_KEY, _levelGoal);
        _settingsChanged = false;
    }
}
bool WaterPump::loadSettings()
{
    if (_memoryProvider != nullptr)
    {
        _levelGoal = _memoryProvider->loadInt(SETTINGS_PUMP_GOAL_KEY, 0);
        return true;
    }
    else
    {
        return false;
    }
}

void WaterPump::onConnectionChange()
{
    if (_sourceIsButton)
    {
        if (firebaseService != nullptr)
            firebaseService->uploadState(FIREBASE_PUMP_CONNECTED_KEY, isConnected());
        _sourceIsButton = false;
    }

    if (isBluetoothRunning())
    {
        std::string s = isConnected() ? "true" : "false";
        if (_connectedCharacteristic != nullptr)
        {
            _connectedCharacteristic->setValue(s);
            _connectedCharacteristic->notify();
        }
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
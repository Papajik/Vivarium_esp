#include "humidifier.h"
#include "../../internal/outlets/outletController.h"
#include "../dht/state_values.h"
#include "../dht/dht.h"

#define CONNECTED_KEY "hum/c"
#define FIREBASE_IS_ON_STATE "/hum/isOn"

#define FCM_KEY "Humidifier"

Humidifier *humidifierPtr = nullptr;

Humidifier::Humidifier(int outlet, MemoryProvider *m, int position) : IModule(CONNECTED_KEY, position, m), _outlet(outlet)
{
    loadSettings();
    if (outletController != nullptr)
    {
        outletController->reserveOutlet(outlet);
    }
}



/// Module
void Humidifier::onLoop()
{
    checkConnectionChange();

    if (_settingsChanged)
    {
        saveSettings();
    }

    if (isConnected())
    {
        //Failsafe
        bool tempConnected;
        stateStorage.getValue(STATE_DHT_CONNECTED, &tempConnected);
        if (!tempConnected)
        {
            setOn(false);
            setConnected(false, true);
            return;
        }

        checkHumidity();
    }
}

void Humidifier::checkHumidity()
{
    printlnD("Humidifier - check humidity");

    float current;
    if (stateStorage.getValue(STATE_DHT_HUMIDITY, &current))
    {
        printV("value = ");
        printlnV(current);
        // Check goal
        if (_humGoal != HUMIDIFIER_INVALID_GOAL && current != DHT_INVALID_VALUE)
        {
            if (current < _humGoal)
            {
                printlnV("Turn outlet on");
                setOn(true);
            }
            else
            {
                printlnV("turn outlet off");
                setOn(false);
            }
        }
    }
    else
    {
        // Fail safe
        setOn(false);
    }
}

void Humidifier::saveSettings()
{
    _memoryProvider->saveFloat(SETTINGS_HUMIDIFIER_GOAL, _humGoal);
    _settingsChanged = false;
}
bool Humidifier::loadSettings()
{
    if (_memoryProvider != nullptr)
        _humGoal = _memoryProvider->loadFloat(SETTINGS_HUMIDIFIER_GOAL, HUMIDIFIER_INVALID_GOAL);

    return false;
}
void Humidifier::onConnectionChange()
{

    sendConnectionChangeNotification("Humidifier", isConnected());
    if (_sourceIsButton)
    {
        firebaseService->uploadState(FIREBASE_HUM_CONNECTED_KEY, isConnected());
        _sourceIsButton = false;
    }

    if (isBluetoothRunning())
    {
        std::string s = isConnected() ? "true" : "false";
        _connectedCharacteristic->setValue(s);
        _connectedCharacteristic->notify();
    }
}

void Humidifier::setGoalHum(float h)
{
    if (h != _humGoal)
    {
        _settingsChanged = true;
        _humGoal = h;
    }
}

float Humidifier::getGoalHum()
{
    return _humGoal;
}

void Humidifier::setOn(bool b)
{
    if (_isOn == b)
        return;
    _isOn = b;
    if (outletController != nullptr)
        outletController->setOutlet(_outlet, b);
    firebaseService->uploadState(FIREBASE_IS_ON_STATE, b);

    if (isBluetoothRunning())
    {
        std::string s = isConnected() ? "true" : "false";
        _connectedCharacteristic->setValue(s);
        _humidifierOnCharacteristic->notify();
    }
};

bool Humidifier::isHumidifierOn()
{
    return _isOn;
}

std::vector<String> Humidifier::getText()
{
    if (!_connected)
    {
        return {"Humidifier", "Disconnected"};
    }
    else
    {
        return {"Humidifier: " + String(_isOn ? "ON" : "OFF"), "Goal: " + String(_humGoal, 2) + " %"};
    }
}
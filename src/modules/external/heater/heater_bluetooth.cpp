#include "heater.h"

//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//Bluetooth
#include <NimBLEDevice.h>

#define SETTINGS_HANDLES 6
#define STATE_HANDLES 6

//Settings
#define CHARACTERISTIC_UUID_GOAL "4BF00501-64C2-4D4F-BF83-5BE82815957F"
#define CHARACTERISTIC_UUID_MODE "4BF00502-64C2-4D4F-BF83-5BE82815957F"

//State
#define CHARACTERISTIC_UUID_MODULE_CONNECTED "4BF00500-64C2-4D4F-BF83-5BE82815957F"
#define CHARACTERISTIC_UUID_CURRENT_POWER "4BF00503-64C2-4D4F-BF83-5BE82815957F"

class SettingsGoalCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsGoalCallbacks(Heater *module)
    {
        h = module;
    }

private:
    Heater *h;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        h->setGoal(parseDouble(pCharacteristic, h->getGoal()));
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        printlnA("SettingsGoalCallbacks - onRead");
        printA("Goal = ");
        printlnA(h->getGoal());
        pCharacteristic->setValue(h->getGoal());
    }
};

class SettingsModeCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsModeCallbacks(Heater *module)
    {
        m = module;
    }

private:
    Heater *m;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        int mode = atoi(pCharacteristic->getValue().c_str());
        m->setMode(Mode(mode));

        // if (mode == Mode::AUTO)
        // {
        //     m->setMode(Mode::AUTO);
        // }
        // else if (mode == Mode::PID)
        // {
        //     m->setMode(Mode::PID);
        // }
        // else if (mode == Mode::TERMO)
        // {
        //     m->setMode(Mode::TERMO);
        // }
        // else
        // {
        //     m->setMode(Mode::UNKNWON);
        // }
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue((int)m->getMode());
    }
};

class StateCurrentPowerCallbacks : public BLECharacteristicCallbacks
{
public:
    StateCurrentPowerCallbacks(Heater *module)
    {
        m = module;
    }

private:
    Heater *m;

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(m->getCurrentPower());
    }
};

void Heater::setupBLESettings(NimBLEService *settings)
{
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_GOAL, new SettingsGoalCallbacks(this));
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_MODE, new SettingsModeCallbacks(this));
}

void Heater::setupBLEState(NimBLEService *state)
{
    setConnectionCallback(state, CHARACTERISTIC_UUID_MODULE_CONNECTED, this);
    _currentPowerCharacteristic = setStateCharacteristic(state, CHARACTERISTIC_UUID_CURRENT_POWER, new StateCurrentPowerCallbacks(this));
}

void Heater::onBLEDisconnect() {}
void Heater::onBLEConnect() {}
void Heater::getHandlesCount(int *settings, int *state, int *credentials)
{
    *settings = SETTINGS_HANDLES;
    *state = STATE_HANDLES;
    *credentials = 0;
}
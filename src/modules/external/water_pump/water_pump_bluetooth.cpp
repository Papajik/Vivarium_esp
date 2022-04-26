#include "water_pump.h"

//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//Bluetooth
#include <NimBLEDevice.h>

#define SETTINGS_HANDLES 3
#define STATE_HANDLES 6

//Settings
#define CHARACTERISTIC_UUID_LEVEL_GOAL "15201001-1AA7-4676-81D1-63F66F402A9C"

//State
#define CHARACTERISTIC_UUID_PUMP_ON "15201002-1AA7-4676-81D1-63F66F402A9C"
#define CHARACTERISTIC_UUID_MODULE_CONNECTED "15201000-1AA7-4676-81D1-63F66F402A9C"

class SettingsGoalCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsGoalCallbacks(WaterPump *module)
    {
        c = module;
    }

private:
    WaterPump *c;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        c->setGoal(atoi(pCharacteristic->getValue().c_str()));
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(c->getGoal());
    }
};

class StateIsOnCallbacks : public BLECharacteristicCallbacks
{
public:
    StateIsOnCallbacks(WaterPump *module)
    {
        c = module;
    }

private:
    WaterPump *c;

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(c->isRunning() ? "true" : "false");
    }
};

void WaterPump::setupBLESettings(NimBLEService *settings)
{
    createSettingsCharacteristic(settings, CHARACTERISTIC_UUID_LEVEL_GOAL, new SettingsGoalCallbacks(this));
}

void WaterPump::setupBLEState(NimBLEService *state)
{
    _pumpRunningCharacteristic = createStateCharacteristic(state, CHARACTERISTIC_UUID_PUMP_ON, new StateIsOnCallbacks(this));
    setConnectionCallback(state, CHARACTERISTIC_UUID_MODULE_CONNECTED, this);
}

void WaterPump::onBLEDisconnect() {}
void WaterPump::onBLEConnect() {}

void WaterPump::getHandlesCount(int *settings, int *state, int *credentials)
{
    *settings = SETTINGS_HANDLES;
    *state = STATE_HANDLES;
}
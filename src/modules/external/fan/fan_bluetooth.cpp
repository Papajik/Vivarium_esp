#include "fan.h"

//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//Bluetooth
#include <NimBLEDevice.h>

#define SETTINGS_HANDLES 6
#define STATE_HANDLES 6

//Settings
#define CHARACTERISTIC_UUID_START_AT "1F100801-EF20-47D2-8A82-26B96D35E25D"
#define CHARACTERISTIC_UUID_MAX_AT "1F100802-EF20-47D2-8A82-26B96D35E25D"

//State
#define CHARACTERISTIC_UUID_MODULE_CONNECTED "1F100800-EF20-47D2-8A82-26B96D35E25D"
#define CHARACTERISTIC_UUID_CURRENT_SPEED "1F100803-EF20-47D2-8A82-26B96D35E25D"

class SettingsStartAtCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsStartAtCallbacks(FanController *module)
    {
        c = module;
    }

private:
    FanController *c;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        c->setStartAt(parseFloat(pCharacteristic, c->getStartAt()));
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(c->getStartAt());
    }
};

class SettingsMaxAtCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsMaxAtCallbacks(FanController *module)
    {
        c = module;
    }

private:
    FanController *c;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        c->setMaxAt(parseFloat(pCharacteristic, c->getMaxAt()));
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(c->getMaxAt());
    }
};

class StateSpeedCallbacks : public BLECharacteristicCallbacks
{
public:
    StateSpeedCallbacks(FanController *module)
    {
        c = module;
    }

private:
    FanController *c;

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(c->getSpeed());
    }
};

void FanController::setupBLESettings(NimBLEService *settings)
{
    createSettingsCharacteristic(settings, CHARACTERISTIC_UUID_MAX_AT, new SettingsMaxAtCallbacks(this));
    createSettingsCharacteristic(settings, CHARACTERISTIC_UUID_START_AT, new SettingsStartAtCallbacks(this));
}

void FanController::setupBLEState(NimBLEService *state)
{
    _currentSpeedCharacteristic = createStateCharacteristic(state, CHARACTERISTIC_UUID_CURRENT_SPEED, new StateSpeedCallbacks(this));
    setConnectionCallback(state, CHARACTERISTIC_UUID_MODULE_CONNECTED, this);
}

void FanController::onBLEDisconnect() {}
void FanController::onBLEConnect() {}

void FanController::getHandlesCount(int *settings, int *state, int *credentials)
{
    *settings = SETTINGS_HANDLES;
    *state = STATE_HANDLES;
    *credentials = 0;
}
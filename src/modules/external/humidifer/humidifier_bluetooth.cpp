#include "humidifier.h"

//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//Bluetooth
#include <NimBLEDevice.h>

#define SETTINGS_HANDLES 3
#define STATE_HANDLES 6

//Settings
#define CHARACTERISTIC_UUID_HUM_GOAL "F1600601-627D-4199-B407-662D33FD77F7"

//State
#define CHARACTERISTIC_UUID_HUM_ON "F1600602-627D-4199-B407-662D33FD77F7"
#define CHARACTERISTIC_UUID_MODULE_CONNECTED "F1600600-627D-4199-B407-662D33FD77F7"

class SettingsHumidityGoalCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsHumidityGoalCallbacks(Humidifier *module)
    {
        c = module;
    }

private:
    Humidifier *c;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        c->setGoalHum(parseFloat(pCharacteristic, c->getGoalHum()));
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(c->getGoalHum());
    }
};

class SettingsHumidifierOnCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsHumidifierOnCallbacks(Humidifier *module)
    {
        c = module;
    }

private:
    Humidifier *c;

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(c->isHumidifierOn() ? "true" : "false");
    }
};

/// Bluetooth
void Humidifier::setupBLESettings(NimBLEService *settings)
{
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_HUM_GOAL, new SettingsHumidityGoalCallbacks(this));
}

void Humidifier::setupBLEState(NimBLEService *state)
{
    _humidifierOnCharacteristic = setStateCharacteristic(state, CHARACTERISTIC_UUID_HUM_ON, new SettingsHumidifierOnCallbacks(this));
    setConnectionCallback(state, CHARACTERISTIC_UUID_MODULE_CONNECTED, this);
}

void Humidifier::onBLEDisconnect() {}
void Humidifier::onBLEConnect() {}

void Humidifier::getHandlesCount(int *settings, int *state, int *credentials)
{
    *settings = SETTINGS_HANDLES;
    *state = STATE_HANDLES;
}
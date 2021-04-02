#include "dht.h"
//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//Bluetooth
#include <NimBLEDevice.h>

#define SETTINGS_HANDLES 12
#define STATE_HANDLES 9

//Settings
#define CHARACTERISTIC_UUID_MAX_T "B2E00903-DEF9-49B8-AEF3-F24CF0147960"
#define CHARACTERISTIC_UUID_MIN_T "B2E00904-DEF9-49B8-AEF3-F24CF0147960"
#define CHARACTERISTIC_UUID_MAX_H "B2E00905-DEF9-49B8-AEF3-F24CF0147960"
#define CHARACTERISTIC_UUID_MIN_H "B2E00906-DEF9-49B8-AEF3-F24CF0147960"

//State
#define CHARACTERISTIC_UUID_MODULE_CONNECTED "B2E00900-DEF9-49B8-AEF3-F24CF0147960"
#define CHARACTERISTIC_UUID_CURRENT_HUMIDITY "B2E00901-DEF9-49B8-AEF3-F24CF0147960"
#define CHARACTERISTIC_UUID_CURRENT_TEMP "B2E00902-DEF9-49B8-AEF3-F24CF0147960"

/// Callbacks
class SettingsMaxHum : public BLECharacteristicCallbacks
{
public:
    SettingsMaxHum(DhtModule *module)
    {
        f = module;
    }

private:
    DhtModule *f;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        f->setMaxHum(parseFloat(pCharacteristic, f->getMaxHum()));
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(f->getMaxHum());
    }
};

class SettingsMinHum : public BLECharacteristicCallbacks
{
public:
    SettingsMinHum(DhtModule *module)
    {
        f = module;
    }

private:
    DhtModule *f;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        f->setMinHum(parseFloat(pCharacteristic, f->getMinHum()));
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(f->getMinHum());
    }
};

class SettingsMaxTemp : public BLECharacteristicCallbacks
{
public:
    SettingsMaxTemp(DhtModule *module)
    {
        f = module;
    }

private:
    DhtModule *f;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        f->setMaxTemp(parseFloat(pCharacteristic, f->getMaxTemp()));
    }
    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(f->getMaxTemp());
    }
};

class SettingsMinTemp : public BLECharacteristicCallbacks
{
public:
    SettingsMinTemp(DhtModule *module)
    {
        f = module;
    }

private:
    DhtModule *f;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        f->setMinTemp(parseFloat(pCharacteristic, f->getMinTemp()));
    }
    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(f->getMinTemp());
    }
};

class StateCurrentTemp : public BLECharacteristicCallbacks
{
public:
    StateCurrentTemp(DhtModule *module)
    {
        f = module;
    }

private:
    DhtModule *f;

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(f->getTemp());
    }
};

class StateCurrentHum : public BLECharacteristicCallbacks
{
public:
    StateCurrentHum(DhtModule *module)
    {
        f = module;
    }

private:
    DhtModule *f;

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(f->getHumidity());
    }
};

void DhtModule::setupBLESettings(NimBLEService *settings)
{
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_MAX_T, new SettingsMaxTemp(this));
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_MIN_T, new SettingsMinTemp(this));
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_MAX_H, new SettingsMaxHum(this));
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_MIN_H, new SettingsMinHum(this));
}

void DhtModule::setupBLEState(NimBLEService *state)
{
    setConnectionCallback(state, CHARACTERISTIC_UUID_MODULE_CONNECTED, this);
    _tempCharacteristic = setStateCharacteristic(state, CHARACTERISTIC_UUID_CURRENT_TEMP, new StateCurrentTemp(this));
    _humidityCharacteristic = setStateCharacteristic(state, CHARACTERISTIC_UUID_CURRENT_HUMIDITY, new StateCurrentHum(this));
}

void DhtModule::onBLEDisconnect() {}

void DhtModule::onBLEConnect() {}

void DhtModule::getHandlesCount(int *settings, int *state, int *credentials)
{
    *settings = SETTINGS_HANDLES;
    *state = STATE_HANDLES;
}
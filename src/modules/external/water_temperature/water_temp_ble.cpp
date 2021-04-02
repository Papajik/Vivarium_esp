#include "water_temp.h"

//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//Bluetooth
#include <NimBLEDevice.h>

#define SETTINGS_HANDLES 6
#define STATE_HANDLES 6

//Settings
#define CHARACTERISTIC_UUID_TEMP_MAX "76B00201-715E-11EB-9439-0242AC130002"
#define CHARACTERISTIC_UUID_TEMP_MIN "76B00202-715E-11EB-9439-0242AC130002"

//State
#define CHARACTERISTIC_UUID_MODULE_CONNECTED "76B00200-715E-11EB-9439-0242AC130002"
#define CHARACTERISTIC_UUID_TEMP_CUR "76B00205-715E-11EB-9439-0242AC130002"

class StateCurrrentTemperatureCallbacks : public BLECharacteristicCallbacks
{
public:
    StateCurrrentTemperatureCallbacks(WaterTempModule *module)
    {
        m = module;
    }

private:
    WaterTempModule *m;

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(m->getCurrentTemperature());
    }
};

class SettingsMinTemperature : public BLECharacteristicCallbacks
{
public:
    SettingsMinTemperature(WaterTempModule *module)
    {
        m = module;
    }

private:
    WaterTempModule *m;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        m->setMinTemperature(parseFloat(pCharacteristic, m->getMinTemperature()));
    }
    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(m->getMinTemperature());
    }
};

class SettingsMaxTemperature : public BLECharacteristicCallbacks
{
public:
    SettingsMaxTemperature(WaterTempModule *module)
    {
        m = module;
    }

private:
    WaterTempModule *m;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        m->setMaxTemperature(parseFloat(pCharacteristic, m->getMaxTemperature()));
    }
    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(m->getMaxTemperature());
    }
};

void WaterTempModule::setupBLESettings(NimBLEService *settings)
{

    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_TEMP_MAX, new SettingsMaxTemperature(this));
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_TEMP_MIN, new SettingsMinTemperature(this));
}

void WaterTempModule::setupBLEState(NimBLEService *state)
{
    setConnectionCallback(state, CHARACTERISTIC_UUID_MODULE_CONNECTED, this);
    _currentTempCharacteristic = setStateCharacteristic(state, CHARACTERISTIC_UUID_TEMP_CUR, new StateCurrrentTemperatureCallbacks(this));
}

void WaterTempModule::onBLEDisconnect()
{
    if (_settingsChanged)
    {
        saveSettings();
    }
}

void WaterTempModule::onBLEConnect()
{
}

void WaterTempModule::getHandlesCount(int *settings, int *state, int *credentials)
{
    *settings = SETTINGS_HANDLES;
    *state = STATE_HANDLES;
    *credentials = 0;
}
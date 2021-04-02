#include "water_level.h"
//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//Bluetooth
#include <NimBLEDevice.h>

#define SETTINGS_HANDLES 9
#define STATE_HANDLES 6

//Settings
#define CHARACTERISTIC_UUID_MAX_LEVEL "1F100301-EF20-47D2-8A82-26B96D35E25D"
#define CHARACTERISTIC_UUID_MIN_LEVEL "1F100302-EF20-47D2-8A82-26B96D35E25D"
#define CHARACTERISTIC_UUID_SENSOR_HEIGHT "1F100303-EF20-47D2-8A82-26B96D35E25D"

//State
#define CHARACTERISTIC_UUID_MODULE_CONNECTED "1F100300-EF20-47D2-8A82-26B96D35E25D"
#define CHARACTERISTIC_UUID_CURRENT_LEVEL "1F100304-EF20-47D2-8A82-26B96D35E25D"

class StateCurrentLevelCallbacks : public BLECharacteristicCallbacks
{
public:
    StateCurrentLevelCallbacks(WaterLevel *module)
    {
        m = module;
    }

private:
    WaterLevel *m;

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(m->getWaterLevel());
    }
};

class SettingsMinLevelCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsMinLevelCallbacks(WaterLevel *module)
    {
        m = module;
    }

private:
    WaterLevel *m;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        m->setMinLevel(atoi(pCharacteristic->getValue().c_str()));
    }
    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(m->getMinLevel());
    }
};

class SettingsMaxLevelCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsMaxLevelCallbacks(WaterLevel *module)
    {
        m = module;
    }

private:
    WaterLevel *m;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        m->setMaxLevel(atoi(pCharacteristic->getValue().c_str()));
    }
    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(m->getMaxLevel());
    }
};

class SettingsSensorHeightCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsSensorHeightCallbacks(WaterLevel *module)
    {
        m = module;
    }

private:
    WaterLevel *m;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        m->setSensorHeight(atoi(pCharacteristic->getValue().c_str()));
    }
    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(m->getSensorHeight());
    }
};

void WaterLevel::setupBLESettings(NimBLEService *settings)
{
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_MAX_LEVEL, new SettingsMaxLevelCallbacks(this));
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_MIN_LEVEL, new SettingsMinLevelCallbacks(this));
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_SENSOR_HEIGHT, new SettingsSensorHeightCallbacks(this));
}
void WaterLevel::setupBLEState(NimBLEService *state)
{
    _waterLevelCharacteristic = setStateCharacteristic(state, CHARACTERISTIC_UUID_CURRENT_LEVEL, new StateCurrentLevelCallbacks(this));
    setConnectionCallback(state, CHARACTERISTIC_UUID_MODULE_CONNECTED, this);
}

void WaterLevel::onBLEDisconnect() {}
void WaterLevel::onBLEConnect() {}
void WaterLevel::getHandlesCount(int *settings, int *state, int *credentials)
{

    *settings = SETTINGS_HANDLES;
    *state = STATE_HANDLES;
    *credentials = 0;
}
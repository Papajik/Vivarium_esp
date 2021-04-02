#include "ph_probe.h"

//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//Bluetooth
#include <NimBLEDevice.h>

//Delays
#include <millisDelay.h>

//Settings

#define CHARACTERISTIC_UUID_PH_MAX "50A00101-4E12-11EB-AE93-0242AC130002"
#define CHARACTERISTIC_UUID_PH_MIN "50A00102-4E12-11EB-AE93-0242AC130002"
#define CHARACTERISTIC_UUID_CONTINUOUS "50A00104-4E12-11EB-AE93-0242AC130002"
#define CHARACTERISTIC_UUID_CONTINUOUS_DELAY "50A00105-4E12-11EB-AE93-0242AC130002"

//State
#define CHARACTERISTIC_UUID_MODULE_CONNECTED "50A00100-4E12-11EB-AE93-0242AC130002"
#define CHARACTERISTIC_UUID_PH_CUR "50A00103-4E12-11EB-AE93-0242AC130002"

#define SETTINGS_HANDLES 12
#define STATE_HANDLES 6

class StateWaterPhCallbacks : public NimBLECharacteristicCallbacks
{
public:
    StateWaterPhCallbacks(PhModule *module)
    {
        m = module;
    }

private:
    PhModule *m;
    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(m->getPhValue());
    }
};

void PhModule::setupBLEState(BLEService *state)
{
    characteristicWaterPh = setStateCharacteristic(state, CHARACTERISTIC_UUID_PH_CUR, new StateWaterPhCallbacks(this));
    setConnectionCallback(state, CHARACTERISTIC_UUID_MODULE_CONNECTED, this);
}

void PhModule::setupBLESettings(BLEService *settings)
{

    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_PH_MAX, new SettingsWaterMaxPh(this));
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_PH_MIN, new SettingsWaterMinPh(this));
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_CONTINUOUS, new SettingsContinuousCallbacks(this));
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_CONTINUOUS_DELAY, new SettingsContinuousDelayCallbacks(this));
};

void PhModule::onBLEDisconnect()
{
    printlnA("PH mode - on disconnect");
    if (settingsChanged)
    {
        saveSettings();
    }
}

void PhModule::getHandlesCount(int *settings_handles, int *state_handles, int *credentials_handles)
{
    *settings_handles = SETTINGS_HANDLES;
    *state_handles = STATE_HANDLES;
    *credentials_handles = 0;
}

SettingsWaterMaxPh::SettingsWaterMaxPh(PhModule *m)
{
    p = m;
}

SettingsWaterMinPh::SettingsWaterMinPh(PhModule *m)
{
    p = m;
}

void SettingsWaterMaxPh::onWrite(BLECharacteristic *pCharacteristic)
{
    p->setMaxPh(parseFloat(pCharacteristic, p->getMaxPh()));
}

void SettingsWaterMaxPh::onRead(BLECharacteristic *pCharacteristic)
{
    pCharacteristic->setValue(p->getMaxPh());
}

void SettingsWaterMinPh::onWrite(BLECharacteristic *pCharacteristic)
{
    p->setMinPh(parseFloat(pCharacteristic, p->getMinPh()));
}

void SettingsWaterMinPh::onRead(BLECharacteristic *pCharacteristic)
{
    pCharacteristic->setValue(p->getMinPh());
}

SettingsContinuousCallbacks::SettingsContinuousCallbacks(PhModule *m)
{
    p = m;
}

void SettingsContinuousCallbacks::onWrite(BLECharacteristic *characteristic)
{

    const char *mode = characteristic->getValue().c_str();
    printlnI(mode);
    if (strcmp(mode, "true") == 0)
    {
        p->setContinuous(true);
    }
    else
    {
        p->setContinuous(false);
    }
}

void SettingsContinuousCallbacks::onRead(BLECharacteristic *characteristic)
{

    if (p->isContinuous())
    {
        characteristic->setValue("true");
    }
    else
    {
        characteristic->setValue("false");
    }
}

SettingsContinuousDelayCallbacks::SettingsContinuousDelayCallbacks(PhModule *m)
{
    p = m;
}

void SettingsContinuousDelayCallbacks::onWrite(BLECharacteristic *characteristic)
{
    int n = atoi(characteristic->getValue().c_str());
    p->setContinuousDelay(n);
}

void SettingsContinuousDelayCallbacks::onRead(BLECharacteristic *characteristic)
{
    characteristic->setValue(p->getContinuousDelay());
}
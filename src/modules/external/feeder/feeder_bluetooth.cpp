#include "feeder.h"

#include "../../../utils/timeHelper.h"

//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//Bluetooth
#include <NimBLEDevice.h>

// TODO Read and write triggers over BLE

#define SETTINGS_HANDLES 12
#define STATE_HANDLES 3

//Settings
#define CHARACTERISTIC_UUID_FEEDER_MODE "88A00701-EEDD-4C73-B88F-1E2538B73E95"

#define CHARACTERISTIC_UUID_FEEDER_ID "88A00702-EEDD-4C73-B88F-1E2538B73E95"
#define CHARACTERISTIC_UUID_FEEDER_TIME "88A00703-EEDD-4C73-B88F-1E2538B73E95"
#define CHARACTERISTIC_UUID_FEEDER_COMMAND "88A00704-EEDD-4C73-B88F-1E2538B73E95"
//State
#define CHARACTERISTIC_UUID_MODULE_CONNECTED "88A00700-EEDD-4C73-B88F-1E2538B73E95"

class SettingsModeCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsModeCallbacks(Feeder *module)
    {
        f = module;
    }

private:
    Feeder *f;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        int mode = atoi(pCharacteristic->getValue().c_str());

        if (mode == FeederMode::BOX)
        {
            f->setMode(FeederMode::BOX);
        }
        else if (mode == FeederMode::SCREW)
        {
            f->setMode(FeederMode::SCREW);
        }
        else
        {
            f->setMode(FeederMode::INVALID);
        }
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue((int)f->getMode());
    }
};


class SettingsFeedTriggerCommandCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsFeedTriggerCommandCallbacks(Feeder *module)
    {
        f = module;
    }

private:
    Feeder *f;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        const char *command = pCharacteristic->getValue().c_str();
        if (strcmp(command, "read") == 0)
        {
            f->uploadTriggerToCharacteristics();
        }

        if (strcmp(command, "delete") == 0)
        {
            f->removeTriggerFromCharacteristic();
        }

        if (strcmp(command, "save") == 0)
        {
            f->parseTriggerFromCharacteristics();
        }
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
    }
};

class SettingsFeedTriggerTime : public BLECharacteristicCallbacks
{
public:
    SettingsFeedTriggerTime(Feeder *module)
    {
        f = module;
    }

private:
    Feeder *f;
};


void Feeder::setupBLESettings(NimBLEService *settings)
{
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_FEEDER_MODE, new SettingsModeCallbacks(this));
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_FEEDER_COMMAND, new SettingsFeedTriggerCommandCallbacks(this));
    _timeCharacteristic = setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_FEEDER_TIME, nullptr);
    _idCharacteristic = setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_FEEDER_ID, nullptr);
}

void Feeder::setupBLEState(NimBLEService *state)
{
    setConnectionCallback(state, CHARACTERISTIC_UUID_MODULE_CONNECTED, this);
}

void Feeder::onBLEDisconnect() {}
void Feeder::onBLEConnect() {}


void Feeder::getHandlesCount(int *settings, int *state, int *credentials)
{
    *settings = SETTINGS_HANDLES;
    *state = STATE_HANDLES;
    *credentials = 0;
}

void Feeder::uploadTriggerToCharacteristics()
{
    std::shared_ptr<FeedTrigger> trigger = findTrigger(String(_idCharacteristic->getValue().c_str()));
    if (trigger != nullptr)
    {
        _timeCharacteristic->setValue(getTime(trigger->hour, trigger->minute));
    }
}

void Feeder::parseTriggerFromCharacteristics()
{
    std::shared_ptr<FeedTrigger> trigger = std::make_shared<FeedTrigger>();
    int time = atoi(_timeCharacteristic->getValue().c_str());
    String firebaseKey = String(_idCharacteristic->getValue().c_str());
    createTrigger(time, firebaseKey);
}

void Feeder::removeTriggerFromCharacteristic() {
    String id = String(_idCharacteristic->getValue().c_str());
    removeTrigger(id);
}
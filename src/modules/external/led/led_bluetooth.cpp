#include "led.h"

//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//Bluetooth
#include <NimBLEDevice.h>

#define SETTINGS_HANDLES 12
#define STATE_HANDLES 6

//Settings
#define CHARACTERISTIC_UUID_LED_ID "1F100401-EF20-47D2-8A82-26B96D35E25D"
#define CHARACTERISTIC_UUID_LED_TIME "1F100402-EF20-47D2-8A82-26B96D35E25D"
#define CHARACTERISTIC_UUID_LED_COLOR "1F100404-EF20-47D2-8A82-26B96D35E25D"
#define CHARACTERISTIC_UUID_LED_COMMAND "1F100405-EF20-47D2-8A82-26B96D35E25D"

//State
#define CHARACTERISTIC_UUID_MODULE_CONNECTED "1F100400-EF20-47D2-8A82-26B96D35E25D"
#define CHARACTERISTIC_UUID_CURRENT_COLOR "1F100403-EF20-47D2-8A82-26B96D35E25D"

class SettingsLedTriggerCommandCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsLedTriggerCommandCallbacks(LedModule *module)
    {
        f = module;
    }

private:
    LedModule *f;
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

class SettingsColorCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsColorCallbacks(LedModule *module)
    {
        c = module;
    }

private:
    LedModule *c;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        c->setColor(strtoul(pCharacteristic->getValue().c_str(), NULL, 0));
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(c->getColor());
    }
};

void LedModule::setupBLESettings(NimBLEService *settings)
{
    _idCharacteristic = setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_LED_ID, nullptr);
    _timeCharacteristic = setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_LED_TIME, nullptr);
    _colorCharacteristic = setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_LED_COLOR, nullptr);
    setSettingsCharacteristic(settings, CHARACTERISTIC_UUID_LED_COMMAND, new SettingsLedTriggerCommandCallbacks(this));
}

void LedModule::setupBLEState(NimBLEService *state)
{
    setConnectionCallback(state, CHARACTERISTIC_UUID_MODULE_CONNECTED, this);
    _currentColorCharacteristic = setStateCharacteristic(state, CHARACTERISTIC_UUID_CURRENT_COLOR, new SettingsColorCallbacks(this));
}

void LedModule::onBLEDisconnect() {}
void LedModule::onBLEConnect() {}
void LedModule::getHandlesCount(int *settings, int *state, int *credentials)
{
    *settings = SETTINGS_HANDLES;
    *state = STATE_HANDLES;
    *credentials = 0;
}

void LedModule::uploadTriggerToCharacteristics()
{
    std::shared_ptr<LedTrigger> trigger = findTrigger(String(_idCharacteristic->getValue().c_str()));
    if (trigger != nullptr)
    {
        _timeCharacteristic->setValue(getTime(trigger->hour, trigger->minute));
        _colorCharacteristic->setValue(trigger->color);
    }
}

void LedModule::parseTriggerFromCharacteristics()
{

    int time = atoi(_timeCharacteristic->getValue().c_str());
    int color = atoi(_colorCharacteristic->getValue().c_str());
    String id = String(_idCharacteristic->getValue().c_str());
    createTrigger(time, color, id);
}

void LedModule::removeTriggerFromCharacteristic()
{
    String id = String(_idCharacteristic->getValue().c_str());
    removeTrigger(id);
}
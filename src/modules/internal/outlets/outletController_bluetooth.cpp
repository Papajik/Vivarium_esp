#include "outletController.h"

//Debug
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//Bluetooth
#include <NimBLEDevice.h>

#define STATE_HANDLES 6

//State
#define CHARACTERISTIC_UUID_PREFIX "E7D0110"
#define CHARACTERISTIC_UUID_SUFFIX "-5513-48D9-8558-8B657B49B801"

class OutletOnCallbacks : public BLECharacteristicCallbacks
{
public:
    OutletOnCallbacks(OutletController *module, int outlet)
    {
        c = module;
        o = outlet;
    }

private:
    OutletController *c;
    int o;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        const char *enable = pCharacteristic->getValue().c_str();
        c->setOutlet(o, strcmp(enable, "true") == 0);
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        pCharacteristic->setValue(c->isOutletOn(o) ? "true" : "false");
    }
};

void OutletController::setupBLESettings(NimBLEService *settings)
{
}
void OutletController::setupBLEState(NimBLEService *state)
{
    for (int i = 0; i < _outletCount; i++)
    {
        if (!_reserved[i])
        {
            String s = String(CHARACTERISTIC_UUID_PREFIX) + String(i) + String(CHARACTERISTIC_UUID_SUFFIX);
            createStateCharacteristic(state, s.c_str(), new OutletOnCallbacks(this, i));
        }
    }
}

void OutletController::onBLEDisconnect()
{
}
void OutletController::onBLEConnect()
{
}
void OutletController::getHandlesCount(int *settings, int *state, int *credentials)
{
    *state = STATE_HANDLES;
}
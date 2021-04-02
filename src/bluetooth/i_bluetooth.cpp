#include "i_bluetooth.h"
#include "bluetooth.h"

#include <NimBLEDevice.h>

#include "../modules/module.h"

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

bool IBluetooth::isBluetoothRunning()
{
    return bleController.isRunning();
}

IsModuleConnectedCallbacks::IsModuleConnectedCallbacks(IModule *m)
{
    module = m;
}

void IsModuleConnectedCallbacks::onRead(NimBLECharacteristic *pCharacteristic)
{
    if (module->isConnected())
    {
        pCharacteristic->setValue("true");
    }
    else
    {
        pCharacteristic->setValue("false");
    }
}

void IsModuleConnectedCallbacks::onWrite(NimBLECharacteristic *pCharacteristic)
{
    const char *enable = pCharacteristic->getValue().c_str();
    module->setConnected(strcmp(enable, "true") == 0, false);
}

void IBluetooth::setConnectionCallback(NimBLEService *service, const char *uuid, IModule *module)
{
    _connectedCharacteristic = service->createCharacteristic(uuid, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_AUTHEN |
                                                                       NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::NOTIFY| NIMBLE_PROPERTY::INDICATE | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_AUTHEN | NIMBLE_PROPERTY::WRITE_ENC);
    _connectedCharacteristic->setCallbacks(new IsModuleConnectedCallbacks(module));
}

NimBLECharacteristic *IBluetooth::setSettingsCharacteristic(NimBLEService *service, const char *uuid, NimBLECharacteristicCallbacks *callbacks)
{
    NimBLECharacteristic *characteristic = service->createCharacteristic(uuid,
                                                                         NIMBLE_PROPERTY::READ |
                                                                             NIMBLE_PROPERTY::READ_AUTHEN |
                                                                             NIMBLE_PROPERTY::READ_ENC |
                                                                             NIMBLE_PROPERTY::WRITE |
                                                                             NIMBLE_PROPERTY::WRITE_AUTHEN |
                                                                             NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::NOTIFY| NIMBLE_PROPERTY::INDICATE);

    // setCallbacks can be called with nullptr
    if (callbacks == nullptr){
        printA(uuid);
        printlnA(" - callbacks is null");
    }
    characteristic->setCallbacks(callbacks);

    return characteristic;
}

NimBLECharacteristic *IBluetooth::setStateCharacteristic(NimBLEService *service, const char *uuid, NimBLECharacteristicCallbacks *callbacks)
{
    NimBLECharacteristic *characteristic = service->createCharacteristic(uuid,
                                                                         NIMBLE_PROPERTY::READ |
                                                                             NIMBLE_PROPERTY::READ_AUTHEN |
                                                                             NIMBLE_PROPERTY::READ_ENC |
                                                                             NIMBLE_PROPERTY::NOTIFY| NIMBLE_PROPERTY::INDICATE);
    characteristic->setCallbacks(callbacks);
    return characteristic;
}

float parseFloat(NimBLECharacteristic *pChar, float d)
{
    char *e;
    float n = strtof(pChar->getValue().c_str(), &e);
    if (*e != '\0')
    {
        printlnA("Conversion error");
        return d;
    }
    else
    {
        printA("Parsed = ");
        printlnA(n);
        return n;
    }
}

double parseDouble(NimBLECharacteristic *pChar, double d)
{
    char *e;
    float n = strtod(pChar->getValue().c_str(), &e);
    if (*e != '\0')
    {
        printlnA("Conversion error");
        return d;
    }
    else
    {
        printA("Parsed = ");
        printlnA(n);
        return n;
    }
}
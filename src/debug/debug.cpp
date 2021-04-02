#include "debug.h"
#include "../buttonControl/moduleControl/moduleControl.h"
#include "../buttonControl/bluetooth/bluetoothControl.h"
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <HardwareSerial.h>

#include "../memory/memory_provider.h"

#include "../settings/settings.h"
#include "../auth/auth.h"
#include "../state/state_values.h"
#include "../wifi/wifiProvider.h"
#include "../modules/external/dht/dht.h"

#include "../modules/external/feeder/feeder.h"

#include <esp_heap_caps.h>
#include <Esp.h>

DhtModule *dhtDebugPointer;

void moduleButtonPressed(int value)
{
    moduleControl.buttonPressed(value);
}

void bluetoothbuttonPressed(int value)
{
    bluetoothControl.buttonPressed(value);
}

void debugPrintSettingsBytes()
{
    printSettingsBytes(settingsStruct);
}

void debugSaveAuth(String str)
{
    auth.setUserId(str);
    auth.onBLEDisconnect();
}

void debugSaveSsid(String str)
{
    wifiProvider.setSsid(str);
}

void debugSavePass(String str)
{
    wifiProvider.setPassphrase(str);
}

void debugFeed()
{
    if (feederPtr != nullptr)
        feederPtr->feed();
}

void unclaim()
{
    auth.setUserId("");
    auth.onBLEDisconnect();
}

void printCurrentMemory()
{
    printlnD("Printing memory");
    printlnD("****************");
    debugD("Number of writes = %d", memoryProvider.writeCount);


    //Settings
    printSettings(settingsStruct);

    //State
    stateStorage.printState();

    printlnD("****************");
}

void printTest(item_value value)
{
    printD("Value = ");
    printlnD(value.i);  
    printlnD("Test test test");
}

void factoryReset()
{
    memoryProvider.factoryReset();
    ESP.restart();
}

void testBluetooth(int v)
{
    float f = v + 0.1f;
    printA("Setting float = ");
    printlnA(f);
    dhtDebugPointer->_humidityCharacteristic->setValue(f);
    dhtDebugPointer->_humidityCharacteristic->notify();
   
}

void setupDebug()
{
    printlnA("Setup debug");

    if (debugAddFunctionVoid("printCurrentMemory", &printCurrentMemory) >= 0)
    {
        debugSetLastFunctionDescription("Print current memory");
    }

    if (debugAddFunctionVoid("feed", &debugFeed) >= 0)
    {
        debugSetLastFunctionDescription("Feed");
    }

    debugAddFunctionVoid("printSettingsBytes", &debugPrintSettingsBytes);
    // debugAddFunctionVoid("testStateStorage", &debugTestStorage);
    // debugAddFunctionVoid("testStateStorage2", &debugTestStorage2);
    debugAddFunctionVoid("unclaim", &unclaim);

    if (debugAddFunctionStr("saveAuth", &debugSaveAuth) >= 0)
    {
        debugSetLastFunctionDescription("To save auth with sufix");
    }

    if (debugAddFunctionStr("saveAuth", &debugSaveSsid) >= 0)
    {
        debugSetLastFunctionDescription("Save SSID");
    }

    if (debugAddFunctionStr("saveAuth", &debugSavePass) >= 0)
    {
        debugSetLastFunctionDescription("Save");
    }

    if (debugAddFunctionVoid("Factory Reset", &factoryReset) >= 0)
    {
        debugSetLastFunctionDescription("Clears NVS and resets ESP");
    }

    if (debugAddFunctionInt("Module button pressed", &moduleButtonPressed) >= 0)
    {
        debugSetLastFunctionDescription("0-6, -1 for release button");
    }

    if (debugAddFunctionInt("Bluetooth button pressed", &bluetoothbuttonPressed) >= 0)
    {
        debugSetLastFunctionDescription("7 for press, -1 for release");
    }

    if (debugAddFunctionInt("testBluetooth", &testBluetooth) >= 0)
    {
        debugSetLastFunctionDescription("testBluetooth");
    }
}

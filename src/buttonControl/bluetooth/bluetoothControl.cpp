#include "bluetoothControl.h"
#include "../bluetooth/bluetoothControl.h"
#include <Arduino.h>
#include "../../firebase/firebase.h"
#include "../../bluetooth/bluetooth.h"
#include "../../wifi/wifiProvider.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

BluetoothControl bluetoothControl;

void BluetoothControl::buttonPressed(int btn)
{
    //Debounce
    if (millis() < BLE_DEBOUNCE_INTERVAL + _lastPressedTime || _lastPressedButton == btn)
    {
        return;
    }

    /// allow only to enable bluetooth when wifi is off
    if (!wifiProvider.isConnected())
    {
        if (!bleController.isRunning())
            bleController.init();
        return;
    }

    if (btn == BLUETOOTH_BUTTON)
    {
        if (bleController.isRunning())
        {
            printlnA("Stopping bluetooth  and starting fireb    ase");
            bleController.stop();
            firebaseService.startFirebase();
        }
        else
        {
            printlnA("Starting bluetooth and stopping firebase");
            firebaseService.stopFirebase();
            bleController.init();
        }
    }

    // //If clicked bluetooth
    // if (btn == BLUETOOTH_BUTTON)
    // {
    //     if (bleController.isRunning())
    //     {
    //         printlnA("Stopping bluetooth  and starting firebase");
    //         bleController.stop();
    //         firebaseService.startFirebase();
    //     }
    //     _lastPressedBLETime = millis();
    // }

    // if (btn == BLUETOOTH_BUTTON_RELEASED && _lastPressedButton == BLUETOOTH_BUTTON && millis() - _lastPressedBLETime > BLUETOOTH_START_DELAY)
    // {
    //     if (!bleController.isRunning())
    //     {
    //         printlnA("Starting bluetooth and stopping firebase");
    //         firebaseService.stopFirebase();
    //         bleController.init();
    //     }
    // }

    // _lastPressedTime = millis();
    // _lastPressedButton = btn;
}
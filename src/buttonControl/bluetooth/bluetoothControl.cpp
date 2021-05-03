#include "bluetoothControl.h"
#include "../bluetooth/bluetoothControl.h"
#include <Arduino.h>
#include "../../firebase/firebase.h"
#include "../../bluetooth/bluetooth.h"
#include "../../wifi/wifiProvider.h"
#include "../../led/ledControl.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

BluetoothControl bluetoothControl;

void BluetoothControl::buttonPressed(int btn)
{
    //Debounce
    if (_lastPressedButton == btn || millis() < BLE_DEBOUNCE_INTERVAL + _lastPressedTime)
    {
        return;
    }

    _lastPressedTime = millis();
    _lastPressedButton = btn;

    printA("Pressed button");
    printlnA(btn);

    if (btn == BLUETOOTH_BUTTON)
    {
        if (bleController.isRunning())
        {
            printlnA("Stopping bluetooth and starting firebse");
            bleController.setStopInFuture();
            firebaseService.setStartInFuture();
            bluetoothOn = false;
        }
        else
        {
            printlnA("Starting bluetooth and stopping firebase");
            firebaseService.setStopInFuture();
            bleController.setStartInFuture();
            bluetoothOn = true;
        }
        ledControl.updateLedStatus();
    }


}

bool BluetoothControl::isBluetoothOn(){
    return bluetoothOn;
}
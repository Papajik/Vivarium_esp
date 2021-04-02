#ifndef _BLUETOOTH_CONTROL_H
#define _BLUETOOTH_CONTROL_H

#define BLUETOOTH_BUTTON 7
#define BLUETOOTH_BUTTON_RELEASED -1

#define BLUETOOTH_START_DELAY 3000
#define BLE_DEBOUNCE_INTERVAL 200

class BluetoothControl
{
public:
    void buttonPressed(int);

private:
    unsigned long _lastPressedTime;
    unsigned long _lastPressedBLETime;
    int _lastPressedButton;

};
extern BluetoothControl bluetoothControl;

#endif
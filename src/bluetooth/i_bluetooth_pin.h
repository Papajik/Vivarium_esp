#ifndef _BLUETOOTH_PIN_INTERFACE_H_
#define _BLUETOOTH_PIN_INTERFACE_H_

class IBluetoothPIN
{
public:
    virtual void hidePIN() = 0;
    virtual void showPIN(int pin) = 0;
};

#endif
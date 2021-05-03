#ifndef _BLUETOOTH_PIN_INTERFACE_H_
#define _BLUETOOTH_PIN_INTERFACE_H_

class IBluetoothPIN
{
public:
    virtual void hidePIN() = 0;
    virtual void showPIN() = 0;
    virtual void setPINToShow(int pin) = 0;

protected:
    int _pin = 0;
    bool _to_display = false;
    bool _pin_displayed = false;
};

#endif
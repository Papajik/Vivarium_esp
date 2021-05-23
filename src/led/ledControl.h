#ifndef _LED_CONTROL_H
#define _LED_CONTROL_H

#include <Arduino.h>

template <uint8_t Size>
class ShiftRegister74HC595;

//TODO

class LedControl
{

public:
    LedControl();
    virtual ~LedControl();

    virtual void setLedOn(uint8_t pin);
    virtual void setLedOff(uint8_t pin);

    virtual uint8_t getLedStatus(uint8_t);

private:
    virtual void updateLedStatus(uint8_t, uint8_t);
    ShiftRegister74HC595<1> *_sr;
};

#endif
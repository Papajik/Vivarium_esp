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
    ~LedControl();

    void updateLedStatus(uint8_t, uint8_t);

private:
    ShiftRegister74HC595<1> *_sr;
};


#endif
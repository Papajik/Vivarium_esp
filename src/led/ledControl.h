#ifndef _LED_CONTROL_H
#define _LED_CONTROL_H

#include <Arduino.h>


enum Mode
{
    M_OUT,
    M_IN
};

class LedControl
{

public:
    LedControl();
    void updateLedStatus(Mode m = M_IN);

private:
    byte _ledByte = 0b00000000;
};

extern LedControl ledControl;

#endif
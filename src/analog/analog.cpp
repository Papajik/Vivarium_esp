#include "analog.h"

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug


void initAnalog()
{
    int i = (int)(log(ADC_RESOLUTION + 1.0) / log(2.0));
    debugA("Analog width = %d bit", i);
    analogSetWidth(i);
}

#include "ledControl.h"

#define MODULES_LATCH_PIN 19
#define MODULES_CLOCK_PIN 23
#define MODULES_DATA_PIN 5
#define MODULES_BRIGHTNESS_PIN 18


#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug


#include "../buttonControl/moduleControl/moduleControl.h"
#include "../buttonControl/bluetooth/bluetoothControl.h"

LedControl ledControl;


LedControl::LedControl()
{
    pinMode(MODULES_LATCH_PIN, OUTPUT);
    pinMode(MODULES_CLOCK_PIN, OUTPUT);
    pinMode(MODULES_DATA_PIN, OUTPUT);
}

void LedControl::updateLedStatus(Mode mode)
{
    _ledByte = 0;
    printA("Module count = ");
    printlnA(moduleControl.moduleCount());

    for (int i = 0; i < moduleControl.moduleCount(); i++)
    {
        if (!moduleControl.isModuleConnected(i))
        {
            bitSet(_ledByte, i);
        }
    }

    if (!bluetoothControl.isBluetoothOn())
    {
        bitSet(_ledByte, 7);
    }

    // Invert bits if shift register mode is INPUT
    if (mode == M_IN)
    {
        _ledByte = ~_ledByte;
    }

    printA("Updating LED status: ");
    printlnA(_ledByte);
    digitalWrite(MODULES_LATCH_PIN, LOW);
    shiftOut(MODULES_DATA_PIN, MODULES_CLOCK_PIN, MSBFIRST, _ledByte);
    digitalWrite(MODULES_LATCH_PIN, HIGH);
}

#include "ledControl.h"

#define MODULES_LATCH_PIN 19
#define MODULES_CLOCK_PIN 23
#define MODULES_DATA_PIN 5
#define MODULES_BRIGHTNESS_PIN 18

#define CHANNEL 2
#define FREQUENCY 50
#define RESOLUTION 8

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#include <ShiftRegister74HC595.h>


LedControl::LedControl()
{
    _sr = new ShiftRegister74HC595<1>(MODULES_DATA_PIN, MODULES_CLOCK_PIN, MODULES_LATCH_PIN);

    ledcSetup(CHANNEL, FREQUENCY, RESOLUTION);
    ledcAttachPin(MODULES_BRIGHTNESS_PIN, CHANNEL);

    ledcWrite(CHANNEL, 0);
}

void LedControl::updateLedStatus(uint8_t position, uint8_t value)
{
    _sr->set(position, value);
}
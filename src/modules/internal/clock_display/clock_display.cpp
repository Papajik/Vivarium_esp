#include "clock_display.h"

#include <HardwareSerial.h>
#include <ErriezRobotDyn4DigitDisplay.h> //https://github.com/Erriez/ErriezRobotDyn4DigitDisplay
#include <millisDelay.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

ClockDisplay::ClockDisplay(int clk, int dio)
{
    _clock = new RobotDyn4DigitDisplay(clk, dio);
    _delay = new millisDelay();
}

void ClockDisplay::begin()
{
    printlnA("Clock display - begin");
    _clock->begin();
    _clock->setBrightness(clockSettings.brigthness);
    _delay->start(clockSettings.refresh_interval);
}

void ClockDisplay::refreshDisplay()
{
    if (_delay->justFinished())
    {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {

            for (int i = 0; i < 4; i++)
            {
                _clock->rawDigit(i, 0x01000000);
                _clock->rawDigit(i, 0x01000000);
                _clock->rawDigit(i, 0x01000000);
                _clock->rawDigit(i, 0x01000000);
            }

            printlnE("ClockDisplay - Failed to obtain time");
            return;
        }
        printlnV(&timeinfo, "%A, %B %d %Y %H:%M:%S");
        _clock->time(timeinfo.tm_hour, timeinfo.tm_min);
        if (clockSettings.refresh_interval != _delay->delay())
        {
            _delay->stop();
            _delay->start(clockSettings.refresh_interval);
        }
        else
        {
            _delay->repeat();
        }
    }
}

void ClockDisplay::setRefreshInterval(unsigned long interval)
{
    if (interval > MIN_INTERVAL_CONSTRAINT && interval < MAX_INTERVAL_CONSTRAINT)
    {
        clockSettings.refresh_interval = interval;
    }
}

ClockDisplay clockDisplay(CLK_PIN, DIO_PIN);
ClockSettings clockSettings = {DEFAULT_CLOCK_BRIGHTNESS, DEFAULT_CLOCK_REFRESH_INTERVAL};
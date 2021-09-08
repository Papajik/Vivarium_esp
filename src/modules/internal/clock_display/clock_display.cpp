#include "clock_display.h"

#include <HardwareSerial.h>
#include <ErriezRobotDyn4DigitDisplay.h> //https://github.com/Erriez/ErriezRobotDyn4DigitDisplay
#include <millisDelay.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <time.h>
#include <freertos/task.h>
#include "../../../utils/timeHelper.h"

ClockDisplay clockDisplay(CLK_PIN, DIO_PIN);
ClockSettings clockSettings = {DEFAULT_CLOCK_BRIGHTNESS, DEFAULT_CLOCK_REFRESH_INTERVAL};

ClockDisplay::ClockDisplay(int clk, int dio)
    : _clock(new RobotDyn4DigitDisplay(clk, dio)),
      _delay(new millisDelay())
{
}

ClockDisplay::~ClockDisplay()
{
    delete _delay;
}

void ClockDisplay::begin()
{
    _clock->begin();
    _clock->setBrightness(clockSettings.brigthness);
    _delay->start(clockSettings.refresh_interval);
    if (!showTime())
    {
        printlnE("ClockDisplay - Failed to obtain time");
        displayInvalid();
    }
}

void ClockDisplay::refreshDisplay()
{
    if (_delay->justFinished())
    {
        if (!showTime())
        {
            displayInvalid();
        }

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

bool ClockDisplay::showTime()
{
    if (getLocalTime(&timeinfo))
    {
        // printlnA(formatTime(timeinfo.tm_hour, timeinfo.tm_min));
        _clock->time(timeinfo.tm_hour, timeinfo.tm_min);
        return true;
    }
    else
    {
        return false;
    }
}

void ClockDisplay::setRefreshInterval(unsigned long interval)
{
    if (interval > MIN_INTERVAL_CONSTRAINT && interval < MAX_INTERVAL_CONSTRAINT)
    {
        clockSettings.refresh_interval = interval;
    }
}

void ClockDisplay::displayInvalid()
{
    printlnA("CLOCK - display invalid time");
    for (int i = 0; i < 4; i++)
    {
        _clock->digit(i, 0);
    }
    _clock->doubleDots(false);
}

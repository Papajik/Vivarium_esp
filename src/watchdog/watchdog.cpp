#include "watchdog.h"
#include <Arduino.h>
#include "../vivarium/vivarium.h"
#include "../utils/taskHealth/taskHealth.h"
#define LOOP_DELAY 120000
Watchdog watchdog;

void watchDogCallback()
{
    watchdog.checkDeadlock();
}

Watchdog::Watchdog()
{
}

callback Watchdog::getCallback()
{
    return watchDogCallback;
}

void Watchdog::addVivarium(Vivarium *v)
{
    _vivarium = v;
}

void Watchdog::checkDeadlock()
{
    if (_vivarium == nullptr)
        return;

    if (millis() > lastDelayTime + LOOP_DELAY)
    {

        if (!_vivarium->isAlive())
        {
            Serial.println("DEADLOCK DETECTED");
            // _vivarium->restart();
        }
    }
}
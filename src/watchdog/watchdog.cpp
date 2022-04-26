/**
* @file watchdog.cpp
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-09-20
* 
* @copyright Copyright (c) 2021
* 
*/

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

    unsigned long now = millis();
    if (now > lastDelayTime + LOOP_DELAY)
    {

        if (!_vivarium->isAlive())
        {
            Serial.printf(" DEADLOCK DETECTED, now: %lu, Last ping alive: %lu\n", now, _vivarium->getLastAlive());
            _vivarium->printState();
            _vivarium->restart();
        }
    }
}
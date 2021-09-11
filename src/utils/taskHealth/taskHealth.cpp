#include <Arduino.h>
#include "taskHealth.h"

#define ALIVE_TTL 30000

TaskHealth::TaskHealth()
{
    mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(mutex);
}

TaskHealth::~TaskHealth()
{
}

unsigned long TaskHealth::getLastAlive()
{
    return _lastAlive;
}


void TaskHealth::pingAlive()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    _lastAlive = millis();
    xSemaphoreGive(mutex);
}


bool TaskHealth::isAlive()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    unsigned long tm = millis();
    if (_lastAlive > tm) _lastAlive = tm; // fix when lastAlive is in the future (concurency)
    bool alive = tm - _lastAlive < ALIVE_TTL;
    xSemaphoreGive(mutex);

    if (alive == false)
    {
        Serial.printf("Is Alive = false on %lu, lastAlive = %lu", tm, _lastAlive);
    }
    return alive;
}
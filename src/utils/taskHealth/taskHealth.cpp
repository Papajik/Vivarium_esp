#include <Arduino.h>
#include "taskHealth.h"

#define ALIVE_TTL 15000

TaskHealth::TaskHealth()
{
    mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(mutex);
}

TaskHealth::~TaskHealth()
{
}

void TaskHealth::pingAlive()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    _lastAlive = millis();
    Serial.println("Alive: " + String(_lastAlive));
    xSemaphoreGive(mutex);
}

bool TaskHealth::isAlive()
{
    unsigned long tm = millis();

    xSemaphoreTake(mutex, portMAX_DELAY);
    bool alive = tm - _lastAlive < ALIVE_TTL;
    xSemaphoreGive(mutex);
    return alive;
}
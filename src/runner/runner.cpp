#include "runner.h"

#include <FreeRTOS.h>
#include <freertos/task.h>

ParallelRunner runner;

void runnerTask(void *parameter)
{
    for (;;)
    {
        callback c = runner.getNextCallback();
        if (c != nullptr)
            c();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

ParallelRunner::ParallelRunner()
{
}

bool ParallelRunner::addCallback(callback c)
{
    if (_callbackCount < MAX_CALLBACKS)
    {
        _callbacks[_callbackCount++] = c;
        return true;
    }
    return false;
}

callback ParallelRunner::getNextCallback()
{
    if (_lastCallback == _callbackCount)
        _lastCallback = 0;
    return _callbacks[_lastCallback++];
}

void ParallelRunner::startRunner()
{
    xTaskCreate(
        runnerTask,       /* Task function. */
        "parallelRunner", /* String with name of task. */
        2200,             /* Stack size in bytes */
        NULL,             /* Parameter passed as input of the task */
        1,                /* Priority of the task.  */
        NULL);            /* Task handle. */
}
#include "monitor.h"
Monitor monitor;
#include <HardwareSerial.h>
#include "../debug/memory.h"
#include "SPIFFS.h"
#include "../vivarium/vivarium.h"

#define CALLBACK_DELAY 10000

char ptrTaskList[600];
unsigned long lastDelay = 0;

void monitorCallback()
{
    monitor.print();
}

void Monitor::print()
{
    if (_vivarium == nullptr)
        return;
    if (lastDelay + CALLBACK_DELAY < millis() && _vivarium->isAlive())
    {
        vTaskList(ptrTaskList);
        Serial.println("****************************************************");
        Serial.println("Task          State   Prio    Stack    Num    Core");
        Serial.println("****************************************************");
        Serial.print(ptrTaskList);
        Serial.println("****************************************************");
        printMemory();
        lastDelay = millis();
        printHeapInfo();
    }
}

callback Monitor::getCallback()
{
    return monitorCallback;
}

void Monitor::addVivarium(Vivarium *v)
{
    _vivarium = v;
}

Monitor::Monitor() {}
Monitor::~Monitor() {}

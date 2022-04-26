/**
* @file monitor.cpp
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-09-20
* 
* @copyright Copyright (c) 2021
* 
*/

#include "monitor.h"
Monitor monitor;
#include <HardwareSerial.h>
#include "../debug/memory.h"
#include "SPIFFS.h"
#include "../../vivarium/vivarium.h"

#define CALLBACK_DELAY 60*1000*5

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

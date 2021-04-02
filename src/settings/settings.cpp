#include "settings.h"

#include <Arduino.h>

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

SettingsStruct *settingsStruct = new SettingsStruct();

void printSettings(SettingsStruct *settings)
{

#ifndef DEBUG_MINIMUM
    printlnD("*** Settings *** ");
    debugD("Led on: %d", settings->ledOn);
    debugD("Led color: %d", settings->ledColor);

    debugD("Max water height: %0.2f", settings->maxWaterHeight);
    debugD("Min water height: %0.2f", settings->minWaterHeight);
    debugD("Max water pH: %0.2f", settings->waterMaxPh);
    debugD("Min water pH: %0.2f", settings->waterMinPh);
    debugD("Optimal water temperature: %0.2f", settings->waterOptimalTemperature);
    debugD("Water sensor height: %0.2f", settings->waterSensorHeight);

    debugD("Heater type: %d", settings->waterHeaterType);

    debugD("Outlet one on: %d", settings->powerOutletOneIsOn);
    debugD("Outlet two on: %d", settings->powerOutletTwoIsOn);

    printlnD("**  Feed Triggers **");
    debugD("Feed triggers count: %d", settings->feedTriggerCount);

    for (int i = 0; i < settings->feedTriggerCount; i++)
    {
        debugD("Feed trigger %d:", i);
        debugD("\tTime: %d", settings->feedTriggers[i].time);
        debugD("\tType: %d", settings->feedTriggers[i].type);
    }

    printlnD("**  Led Triggers **");
    debugD("LED triggers count: %d", settings->ledTriggerCount);

    for (int i = 0; i < settings->ledTriggerCount; i++)
    {
        debugD("LED trigger %d:", i);
        debugD("\tTime: %d", settings->ledTriggers[i].time);
        debugD("\tColor: %d", settings->ledTriggers[i].color);
    }

    printlnD("**  Outlet 1 Triggers **");
    debugD("triggers count: %d", settings->outletOneTriggerCount);

    for (int i = 0; i < settings->outletOneTriggerCount; i++)
    {
        debugD("Outlet 1 trigger %d:", i);
        debugD("\tTime: %d", settings->outletOneTriggers[i].time);
        debugD("\tTurn on: %d", settings->outletOneTriggers[i].turnOn);
    }

    printlnD("**  Outlet 2 Triggers **");
    debugD("triggers count: %d", settings->outletTwoTriggerCount);

    for (int i = 0; i < settings->outletTwoTriggerCount; i++)
    {
        debugD("Outlet 2 trigger %d:", i);
        debugD("\tTime: %d", settings->outletTwoTriggers[i].time);
        debugD("\tTurn on: %d", settings->outletTwoTriggers[i].turnOn);
    }

#endif //DEBUG_MINIMUM
}

void clearSettings(SettingsStruct *settings)
{
    memset(settings, 0, sizeof(&settings));
}

void printSettingsBytes(SettingsStruct *settings)
{

    
    printlnD("Printing array");
    unsigned char *ptr = (unsigned char *)settings;
    const unsigned char *byte;
    size_t size = sizeof(SettingsStruct);
    for (byte = ptr; size--; ++byte)
    {
        Serial.printf("%02X", *byte);
        Serial.print(" ");
    }
    Serial.println("");
}
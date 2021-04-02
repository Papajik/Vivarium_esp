#ifndef _SETTINGS_H_
#define _SETTINGS_H_
#include <Arduino.h>

//Extern
#define MAX_TRIGGERS_FEEDER 2
#define MAX_TRIGGERS_LED 2
#define MAX_TRIGGERS_OUTLETS 2

#define LED_TRIGGER_BYTE_SIZE 6
#define FEED_TRIGGER_BYTE_SIZE 3
#define OUTLET_TRIGGER_BYTE_SIZE 3




struct LedTriggerStruct
{                 // 6 bytes
  uint16_t time;  // 2 - 1. byte -> hour, 2. byte -> minute
  uint32_t color; // 4
};

struct FeedTriggerStruct
{
  uint16_t time; // 2 bytes
  uint8_t type;  // 1 byte
};

struct OutletTriggerStruct
{
  uint16_t time; // 2 bytes
  bool turnOn;   // 1 byte
};

struct firmwareSetings
{

  uint32_t clock_interval;        //clock_display.h
  uint32_t clock_brightness;      //clock_display.h
};

struct SettingsStruct
{
  bool ledOn;                                                  // 1 byte
  uint32_t ledColor;                                           // 4 bytes
  float maxWaterHeight;                                        // 4 bytes
  float minWaterHeight;                                        // 4 bytes
  bool powerOutletOneIsOn;                                     // 1 byte
  bool powerOutletTwoIsOn;                                     // 1 byte
  uint8_t waterHeaterType;                                     // 1 bytes
  float waterMaxPh;                                            // 4 bytes
  float waterMinPh;                                            // 4 bytes
  float waterOptimalTemperature;                               // 4 bytes
  float waterSensorHeight;                                     // 4 bytes
  uint8_t feedTriggerCount;                                    // 1 byte
  FeedTriggerStruct feedTriggers[MAX_TRIGGERS_FEEDER];         //6*10
  uint8_t ledTriggerCount;                                     // 1 byte
  LedTriggerStruct ledTriggers[MAX_TRIGGERS_LED];              //3*10
  uint8_t outletOneTriggerCount;                               // 1 byte
  OutletTriggerStruct outletOneTriggers[MAX_TRIGGERS_OUTLETS]; //3*10
  uint8_t outletTwoTriggerCount;                               // 1 byte
  OutletTriggerStruct outletTwoTriggers[MAX_TRIGGERS_OUTLETS]; //3*10
};

extern SettingsStruct *settingsStruct;

void printSettings(SettingsStruct *settings);
void clearSettings(SettingsStruct *settings);
void printSettingsBytes(SettingsStruct *settings);

#endif
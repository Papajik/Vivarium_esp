/**
* @file feeder.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/

#ifndef _FEEDER_H
#define _FEEDER_H

#include "../../module.h"

#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"
#include "../../../modules/internal/lcd_display/textModule.h"
#include "../../../modules/alarm/plainAlarm.h"
#include <TimeAlarms.h>
#include <map>
#include <memory>

#include <FreeRTOS.h>
#include <freertos/semphr.h>

#define FEEDER_MODE_1_RPM 15
#define FEEDER_MODE_2_RPM 15
#define FEEDER_STEPS_PER_REVOLUTION 2048
#define FEEDER_IN_1 0
#define FEEDER_IN_2 13
#define FEEDER_IN_3 14
#define FEEDER_IN_4 27
#define FEEDER_MAX_TRIGERS 10

#define SETTINGS_FEEDER_KEY "feeder"

#define FIREBASE_FEEDER_CONNECTED_KEY "/feeder/connected"

#define INVALID_MEMORY_ID -1

class Stepper;

enum FeederMode
{
    SCREW = 0,
    BOX = 1,
    INVALID = -1
};

/**
* @brief Mostly used to feed fish. Has two modes. Supports triggers
* 
*/
struct FeederSettings
{
    FeederMode mode;
};

class Feeder : public IModule,
               public IFirebaseModule,
               public IBluetooth,
               public TextModule,
               public PlainAlarm
{
public:
    Feeder(int, MemoryProvider *, int in_1 = FEEDER_IN_1, int in_2 = FEEDER_IN_2, int in_3 = FEEDER_IN_3, int in_4 = FEEDER_IN_4);
    ~Feeder();

    /// Firebase
    virtual void parseJson(FirebaseJson *, String);
    virtual String getSettingKey();
    virtual void parseValue(String key, String value);
    virtual bool updateSensorData(FirebaseJson *);

    FeederMode getMode();
    void setMode(FeederMode);
    void feed();

    /// Bluetooth
    virtual void setupBLESettings(NimBLEService *settings);
    virtual void setupBLEState(NimBLEService *state);

    virtual void onBLEDisconnect();
    virtual void onBLEConnect();
    virtual void getHandlesCount(int *settings, int *state, int *credentials);

    void uploadTriggerToCharacteristics();
    void parseTriggerFromCharacteristics();
    void removeTriggerFromCharacteristic();

    int getLastFeeded();
    bool feededRecently();
    /// LCD
    std::vector<String> getText();

private:
    int _lastFeededTime = 0;

    NimBLECharacteristic *_timeCharacteristic = nullptr;
    NimBLECharacteristic *_idCharacteristic = nullptr;

    bool _feeded = false;
    FeederSettings _settings;
    std::shared_ptr<Stepper> _stepper;

    /// Module
    virtual void onConnectionChange();
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();

    bool _settingsChanged = false;
};

extern Feeder *feederPtr;

#endif
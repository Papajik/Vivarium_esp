/**
* @file water_temp.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef _WATER_TEMP_H
#define _WATER_TEMP_H

#define WATER_TEMP_PIN 4

#define WATER_TEMP_INVALID_VALUE -127

#define SETTINGS_WATER_TEMP_KEY "waterTemp"

#define FIREBASE_WT_CONNECTED_KEY "/waterTemp/connected"

#include "../../module.h"
#include "../../../bluetooth/i_bluetooth.h"
#include "../../../firebase/i_FirebaseModule.h"
#include "../../../modules/internal/lcd_display/textModule.h"

class DallasTemperature;
class millisDelay;
class OneWire;

struct WaterTempSettings
{
    float max_temp;
    float min_temp;
};

class WaterTempModule
    : public IModule,
      public IBluetooth,
      public IFirebaseModule,
      public TextModule
{
public:
    WaterTempModule(int, MemoryProvider *, int pin = WATER_TEMP_PIN);
    ~WaterTempModule();
    void setMinTemperature(float);
    void setMaxTemperature(float);

    void writeTemp(float);

    float getMinTemperature();
    float getMaxTemperature();

    float getCurrentTemperature();

    void startReadings();
    void stopReading();

    /// LCD
    std::vector<String> getText();

    /// Bluetooth
    virtual void setupBLESettings(NimBLEService *settings);
    virtual void setupBLEState(NimBLEService *state);
    virtual void onBLEDisconnect();
    virtual void onBLEConnect();
    virtual void getHandlesCount(int *settings, int *state, int *credentials);

    /// Firebase
    virtual void parseJson(FirebaseJson *, String);
    virtual String getSettingKey();
    virtual void parseValue(String, String);
    virtual bool updateSensorData(FirebaseJson *);
    void readTemperature();

private:
    int _invalidReadingInRow = 0;
    int _lastMessage = 0;
    int _messageCount = 0;
    unsigned long _lastMessageTime = 0;
    float _currentTemp = WATER_TEMP_INVALID_VALUE;
    NimBLECharacteristic *_currentTempCharacteristic;
    OneWire *_oneWire;
    DallasTemperature *_dallas;
    millisDelay *_delay;

    WaterTempSettings _settings;

    /// Module
    virtual void onConnectionChange();
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();

    void checkBoundaries();
    bool _settingsChanged = false;
};

#endif
#ifndef _WATER_TEMP_H
#define _WATER_TEMP_H

#define WATER_TEMP_PIN 4
#define WATER_TEMP_READ_DELAY 500
#define WATER_TEMP_INVALID_VALUE -127

#define SETTINGS_WATER_TEMP_KEY "waterTemp"

#define FIREBASE_WT_CONNECTED_KEY "/waterTemp/connected"

#include "../../module.h"
#include "../../../bluetooth/i_bluetooth.h"
#include "../../../firebase/i_FirebaseModule.h"

class DallasTemperature;
class millisDelay;
class OneWire;

struct WaterTempSettings
{
    float max_temp;
    float min_temp;
};

class WaterTempModule : public IModule, public IBluetooth, public IFirebaseModule
{
public:
    WaterTempModule();
    ~WaterTempModule();
    void setMinTemperature(float);
    void setMaxTemperature(float);

    float getMinTemperature();
    float getMaxTemperature();

    float getCurrentTemperature();

    void startReadings();
    void stopReading();

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
    virtual void updateSensorData(FirebaseJson *);
    void readTemperature();

    virtual bool isFModule() { return true; }
    virtual bool isBModule() { return true; }

private:
    float _currentTemp = WATER_TEMP_INVALID_VALUE;
    NimBLECharacteristic *_currentTempCharacteristic;
    DallasTemperature *_dallas;
    OneWire *_oneWire;
    WaterTempSettings _settings;
    millisDelay *_delay;

    /// Module
    virtual void onConnectionChange();
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();

    void checkBoundaries();
    bool _settingsChanged = false;
};

#endif
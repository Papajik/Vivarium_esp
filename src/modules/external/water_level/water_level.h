#ifndef _WATER_LEVEL_H_
#define _WATER_LEVEL_H_

#include "../../module.h"
#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"

#define SETTINGS_WL_KEY "wl"
#define FIREBASE_WL_CONNECTED_KEY "/wl/connected"

#define W_LEVEL_ECHO_PIN 33
#define W_LEVEL_TRIG_PIN 32

class millisDelay;

class NewPing;

struct WaterLevelSettings
{
    int minLevel;
    int maxLevel;
    int sensorHeight;
};

class WaterLevel : public IModule, public IFirebaseModule, public IBluetooth
{
public:
    WaterLevel(int, int echo = W_LEVEL_ECHO_PIN, int trig = W_LEVEL_TRIG_PIN);
    void setMaxLevel(int);
    void setMinLevel(int);
    void setSensorHeight(int);
    int getMinLevel();
    int getMaxLevel();
    int getSensorHeight();
    int getWaterLevel();

    void setLevel(int);

    void checkBoundaries();

    /// Firebase
    virtual void parseJson(FirebaseJson *, String);
    virtual String getSettingKey();
    virtual void parseValue(String, String);
    virtual void updateSensorData(FirebaseJson *);

    /// Bluetooth

    virtual void setupBLESettings(NimBLEService *settings);
    virtual void setupBLEState(NimBLEService *state);

    virtual void onBLEDisconnect();
    virtual void onBLEConnect();
    virtual void getHandlesCount(int *settings, int *state, int *credentials);

private:
    NimBLECharacteristic *_waterLevelCharacteristic;
    int _waterLevel;
    millisDelay *_delay;
    WaterLevelSettings _settings;
    bool _settingsChanged = false;
    NewPing *_sonar;

    void readLevel();

    /// Module
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();
    virtual void onConnectionChange();
};

#endif
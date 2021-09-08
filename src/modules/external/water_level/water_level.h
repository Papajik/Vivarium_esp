#ifndef _WATER_LEVEL_H_
#define _WATER_LEVEL_H_

#include "../../module.h"
#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"
#include "../../../modules/internal/lcd_display/textModule.h"

#define SETTINGS_WL_KEY "wl"
#define FIREBASE_WL_CONNECTED_KEY "/wl/connected"
// #define READ_CUT 1

#define W_LEVEL_ECHO_PIN 33
#define W_LEVEL_TRIG_PIN 32

#define READ_COUNT 6
#define READ_CUT 2
class millisDelay;

class NewPing;

struct WaterLevelSettings
{
    int minLevel;
    int maxLevel;
    int sensorHeight;
};

class WaterLevel
    : public IModule,
      public IFirebaseModule,
      public IBluetooth,
      public TextModule
{
public:
    WaterLevel(int, int echo = W_LEVEL_ECHO_PIN, int trig = W_LEVEL_TRIG_PIN);
    ~WaterLevel();
    void setMaxLevel(int);
    void setMinLevel(int);
    void setSensorHeight(int);
    int getMinLevel();
    int getMaxLevel();
    int getSensorHeight();
    int getWaterLevel();

    void setLevel(int);

    void checkBoundaries();

    /// LCD
    std::vector<String> getText();

    /// Firebase
    virtual void parseJson(FirebaseJson *, String);
    virtual String getSettingKey();
    virtual void parseValue(String, String);
    virtual bool updateSensorData(FirebaseJson *);

    /// Bluetooth

    virtual void setupBLESettings(NimBLEService *settings);
    virtual void setupBLEState(NimBLEService *state);

    virtual void onBLEDisconnect();
    virtual void onBLEConnect();
    virtual void getHandlesCount(int *settings, int *state, int *credentials);

private:
    int _levelBuffer[READ_COUNT];
    unsigned long _lastValueChange = 0;

    int _lastMessage = 0;
    int _messageCount = 0;
    std::vector<int> _levels;
    NimBLECharacteristic *_waterLevelCharacteristic = nullptr;
    int _waterLevel = 0;
    millisDelay *_delay = nullptr;
    WaterLevelSettings _settings;
    bool _settingsChanged = false;
    NewPing *_sonar = nullptr;

    void readLevel();

    /// Module
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();
    virtual void onConnectionChange();
};

#endif
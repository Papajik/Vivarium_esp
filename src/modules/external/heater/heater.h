#ifndef _HEATER_H_
#define _HEATER_H_

#include "../../module.h"
#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"
#include "../../../modules/internal/lcd_display/textModule.h"
#include "../../../modules/alarm/payloadAlarm.h"

#define SETTINGS_HEATER_KEY "heater"
#define FIREBASE_HEATER_CONNECTED_KEY "/heater/connected"
#define KEY_HEATER_GOAL "/heater/goal"

#define HEATER_PIN 2
#define HEATER_SYNC_PIN 15
#define GOAL_INVALID -1

// #define HEATER_KP 38.81978
// #define HEATER_KI 0.467478
// #define HEATER_KD 80.59064

///Works relatively fine
// #define HEATER_KP 38.81978
// #define HEATER_KI 0.23
// #define HEATER_KD 80.59064

#define HEATER_KP 80
#define HEATER_KI 0.2
#define HEATER_KD 80

class AutoPID;
class dimmerLamp;

enum Mode
{
    PID = 0,
    AUTO = 1,
    THERMO = 2,
    UNKNWON = -1
};

String modeToString(Mode m);

struct HeaterSettings
{
    Mode mode;
    double tempGoal; //temp goal
};

class Heater : public IModule,
               public IFirebaseModule,
               public IBluetooth,
               public TextModule,
               public PayloadAlarm<double>
{
public:
    Heater(int, MemoryProvider *provider, int pwm = HEATER_PIN, int sync = HEATER_SYNC_PIN);

    virtual void beforeShutdown();

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

    void setMode(Mode);
    Mode getMode();

    void runPID();

    void runThermo();
    void updatePower();

    double getGoal();
    void setGoal(double, bool forced = false);
    void setFutureGoal(double);

    double getCurrentPower();

    /// LCD
    std::vector<String> getText();

    void triggerCallback();

private:
    bool checkTemperatureConnected();
    void checkFutureGoal();

    void stop();

    double _oldPower = -1;
    double _futureGoal = -1;
    NimBLECharacteristic *_currentGoalCharacteristic;
    NimBLECharacteristic *_currentPowerCharacteristic;
    bool _settingsChanged = false;
    dimmerLamp *_dimmer = nullptr;
    AutoPID *_pid = nullptr;
    HeaterSettings _settings;
    double _currentTemperature = 0;
    // double _tempGoal;
    double _currentPower = 0;
    unsigned long _lastValidTemp = 0;

    bool failSafeCheck();
    bool loadTemp();

    /// Module
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();
    virtual void onConnectionChange();

    // Alarm
    virtual bool getPayloadFromJson(FirebaseJson *, double &);
    virtual bool getPayloadFromValue(String key, String value, double &);
};
extern Heater *heaterPtr;
#endif
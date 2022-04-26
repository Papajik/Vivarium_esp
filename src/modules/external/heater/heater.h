/**
* @file heater.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/

#ifndef _HEATER_H_
#define _HEATER_H_

#include "../../module.h"
#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"
#include "../../../modules/internal/lcd_display/textModule.h"
#include "../../../modules/alarm/payloadAlarm.h"

#include "pid/pid.h"

#define SETTINGS_HEATER_KEY "heater"
#define FIREBASE_HEATER_CONNECTED_KEY "/heater/connected"
#define KEY_HEATER_GOAL "/heater/goal"

// Upload state
#define KEY_HEATER_KI "/heater/Ki"
#define KEY_HEATER_KP "/heater/Kp"
#define KEY_HEATER_KD "/heater/Kd"
#define KEY_HEATER_PON "/heater/pOn"

#define HEATER_PIN 2
#define HEATER_SYNC_PIN 15
#define GOAL_INVALID -1

class dimmerLamp;
class millisDelay;

enum Mode
{
    PID = 0,    // QuickPID
    AUTO = 1,   // External heater - just let 100 % of power into socket
    THERMO = 2, // ON-OFF
    DIRECT = 3, // Setup power output directly - can have various uses
    UNKNWON = -1 
};

String modeToString(Mode m);

struct HeaterSettings
{
    Mode mode;
    float tempGoal;
    float directPower;
};

/**
* @brief Heater module. Used to raise temperature. Has multiple modes and supports payload alarms.
* 
*/
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
    Mode getMode() { return _settings.mode; };

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

    void printPidSettings();

private:
    bool checkTemperatureConnected();
    void checkFutureGoal();

    void stop();

    double _oldPower = -1;
    float _currentTemperature = 0;
    float _currentPower = 0;
    float _futureGoal = -1;

    millisDelay *_thermoDelay;

    HeaterPID *_pid;

    NimBLECharacteristic *_currentGoalCharacteristic;
    NimBLECharacteristic *_currentPowerCharacteristic;
    bool _settingsChanged = false;
    dimmerLamp *_dimmer = nullptr;

    HeaterSettings _settings;

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
#ifndef _HEATER_H_
#define _HEATER_H_

#include "../../module.h"
#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"

#define SETTINGS_HEATER_KEY "heater"
#define FIREBASE_HEATER_CONNECTED_KEY "/heater/connected"

#define HEATER_PIN 2
#define HEATER_SYNC_PIN 15
#define GOAL_INVALID -1
class AutoPID;
class dimmerLamp;

enum Mode
{
    PID = 0,
    AUTO = 1,
    UNKNWON = -1
};

struct HeaterSettings
{
    Mode mode;
    double tempGoal; //temp goal
};

class Heater : public IModule, public IFirebaseModule, public IBluetooth
{
public:
    Heater(int, int pwm = HEATER_PIN, int sync = HEATER_SYNC_PIN);

    virtual void beforeShutdown();

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

    void setMode(Mode);
    Mode getMode();

    void runPID();
    void setPower();

    double getGoal();
    void setGoal(double);

    double getCurrentPower();

private:
    double _oldPower = 0;

    NimBLECharacteristic *_currentPowerCharacteristic;
    bool _settingsChanged = false;
    dimmerLamp *_dimmer;
    AutoPID *_pid;
    HeaterSettings _settings;
    double _currentTemperature;
    // double _tempGoal;
    double _currentPower = 0;
    unsigned long _lastValidTemp = 0;

    void failSafeCheck();

    /// Module
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();
    virtual void onConnectionChange();
};

#endif
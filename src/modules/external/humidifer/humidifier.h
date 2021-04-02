#ifndef _HUMIDIFIER_H
#define _HUMIDIFIER_H

#include "../../module.h"
#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"

#define FIREBASE_HUM_CONNECTED_KEY "/hum/connected"

#define SETTINGS_HUMIDIFIER_KEY "hum"

#define SETTINGS_HUMIDIFIER_GOAL "humGoal"
#define SETTINGS_HUMIDIFIER_MIN "humMin"
#define SETTINGS_HUMIDIFIER_MAX "humMax"

#define HUMIDIFIER_INVALID_GOAL -1

#define HUMIDIFIER_FAILSAFE_DELAY 10000 // 10 seconds

class Humidifier : public IModule, public IFirebaseModule, public IBluetooth
{
public:
    Humidifier(int);
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

    void setGoalHum(float);
    float getGoalHum();

    bool isHumidifierOn();

    virtual bool isFModule() { return true; }
    virtual bool isBModule() { return true; }

private:
    unsigned long _lastValidTemp = 0;

    void setOn(bool);

    bool _isOn;
    bool _settingsChanged;
    int _outlet;

    float _humGoal;

    NimBLECharacteristic *_humidifierOnCharacteristic;

    void checkHumidity();
    void failSafeCheck();

    /// Module
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();
    virtual void onConnectionChange();
};

#endif
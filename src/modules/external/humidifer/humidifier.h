/**
* @file humidifier.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef _HUMIDIFIER_H
#define _HUMIDIFIER_H

#include "../../module.h"
#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"
#include "../../../modules/internal/lcd_display/textModule.h"

#define FIREBASE_HUM_CONNECTED_KEY "/hum/connected"

#define SETTINGS_HUMIDIFIER_KEY "hum"

#define SETTINGS_HUMIDIFIER_GOAL "humGoal"
#define SETTINGS_HUMIDIFIER_MIN "humMin"
#define SETTINGS_HUMIDIFIER_MAX "humMax"

#define HUMIDIFIER_INVALID_GOAL -1


class Humidifier : public IModule, public IFirebaseModule, public IBluetooth, public TextModule
{
public:
    Humidifier(int, MemoryProvider *, int);
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

    void setGoalHum(float);
    float getGoalHum();

    bool isHumidifierOn();

    /// LCD
    std::vector<String> getText();

private:
    void setOn(bool);

    bool _isOn = false;
    bool _settingsChanged = false;
    int _outlet;

    float _humGoal;

    NimBLECharacteristic *_humidifierOnCharacteristic;

    void checkHumidity();

    /// Module
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();
    virtual void onConnectionChange();
};

extern Humidifier *humidifierPtr;

#endif
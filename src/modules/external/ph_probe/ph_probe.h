#ifndef _PH_PROBE_H_
#define _PH_PROBE_H_

#include "../../module.h"
#include "../../../bluetooth/i_bluetooth.h"
#include "../../../firebase/i_FirebaseModule.h"

class millisDelay;
#include <NimBLEDevice.h>

// Firmware & calibration settings
#define PH_READING_COUNT 10
#define PH_CALIBRATION 23.57
#define PH_VALUES_CUT 2
#define PH_VOLTAGE_CHANGE -5.70
#define PH_PIN 39
#define PH_TIMER_ONCE_DELAY 15
#define PH_TIMER_CONTINUOUS_DELAY 20

#define PH_INVALID_VALUE -1

// Default values for settings
#define PH_DEFAULT_CONTINUOUS_DELAY 60000 // 1 minute

// Key to store settings
#define SETTINGS_PH_KEY "ph"

#define FIREBASE_PH_CONNECTED_KEY "/ph/connected"

struct PhModuleSettings
{
    bool continuous;
    unsigned long continuous_delay;
    float max_ph;
    float min_ph;
};

class PhModule : public IModule, public IBluetooth, public IFirebaseModule
{

public:
    PhModule();

    /*
    @brief Checks if continuous mode is on and if delay already passed. Reads water pH if both assumptions are met. Should be called from main loop.
    */
    void checkContinuousScan();

    /*
    @brief Starts scanning with given mode. 
    @param mode Continous for repeated scan. Single for only one scan. 
    */
    void startScan();
    /*
    @brief Disables scanning. No pH scan will be performed until startScan(mode) is called again
    */
    void stopScan();

    /*
    @override
    */
    void onConnectionChange();

    virtual void saveSettings();
    virtual bool loadSettings();

    void setMinPh(float);
    void setMaxPh(float);

    void setContinuousDelay(int);
    int getContinuousDelay();

    float getMinPh();
    float getMaxPh();

    void setContinuous(bool);
    bool isContinuous();

    float getPhValue();

    virtual void setupBLEState(BLEService *);
    virtual void setupBLESettings(BLEService *);
    virtual void onBLEDisconnect();
    virtual void onBLEConnect();
    virtual void getHandlesCount(int *, int *, int *);

    virtual void onLoop();


    //Bluetooth
    BLECharacteristic *characteristicWaterPh;

    void setSettingsChanged(bool);
    bool settingsChanged = false;

    //Firebase
    virtual void parseJson(FirebaseJson *, String);
    virtual String getSettingKey();
    virtual void parseValue(String key, String value);
    virtual void updateSensorData(FirebaseJson *);

    virtual bool isFModule() { return true; }
    virtual bool isBModule() { return true; }

private:
    void checkBoundaries();
    float _phReadBuffer[PH_READING_COUNT];
    float _readPh();
    float _lastPhValue = -1;
    void _setPhValue(float);
    millisDelay *_delay;

    //Bluetooth
    BLECharacteristic *_characteristicMode;
    PhModuleSettings _settings;

    //WiFi
    // EmergencyCallback _emergencyCallback;
};

//Bluetooth

class SettingsWaterMaxPh : public BLECharacteristicCallbacks
{
public:
    SettingsWaterMaxPh(PhModule *);

private:
    PhModule *p;
    void onWrite(BLECharacteristic *pCharacteristic);
    void onRead(BLECharacteristic *pCharacteristic);
};

class SettingsWaterMinPh : public BLECharacteristicCallbacks
{
public:
    SettingsWaterMinPh(PhModule *);

private:
    PhModule *p;
    void onWrite(BLECharacteristic *);
    void onRead(BLECharacteristic *);
};

class SettingsContinuousCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsContinuousCallbacks(PhModule *);

private:
    PhModule *p;
    void onWrite(BLECharacteristic *);
    void onRead(BLECharacteristic *);
};

class SettingsContinuousDelayCallbacks : public BLECharacteristicCallbacks
{
public:
    SettingsContinuousDelayCallbacks(PhModule *);

private:
    PhModule *p;
    void onWrite(BLECharacteristic *);
    void onRead(BLECharacteristic *);
};

#endif
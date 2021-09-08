#ifndef _FAN_H_
#define _FAN_H_

#define FAN_PIN 12

#include "../../module.h"
#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"
#include "../../../modules/internal/lcd_display/textModule.h"

#define SETTINGS_FAN_KEY "fan"

#define FIREBASE_FAN_CONNECTED_KEY "/fan/connected"

#define FAN_STOP_SPEED 0

#define FAN_MAX_SPEED 255
#define FAN_MIN_SPEED 100


struct FanSettings
{
    float setStartAt;
    float setMaxAt;
};

class FanController : public IModule, public IFirebaseModule, public IBluetooth, public TextModule
{
public:
    FanController(int, int pin = FAN_PIN);
    void setStartAt(float);
    void setMaxAt(float);
    float getMaxAt();
    float getStartAt();

    int getSpeed();

    /// Bluetooth
    // virtual void setupBLESettings(NimBLEService *settings);
    // virtual void setupBLEState(NimBLEService *state);
    // virtual void onBLEDisconnect();
    // virtual void onBLEConnect();
    // virtual void getHandlesNum(int *settings, int *state, int *credentials);

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

    /// LCD
    std::vector<String> getText();

private:
    float getPercantage(int i);
    NimBLECharacteristic *_currentSpeedCharacteristic = nullptr;
    FanSettings _settings;
    bool _settingsChanged = false;
    void parseFanSpeed(float temperature);
    void setSpeed(int speed);
    int _currentSpeed = FAN_STOP_SPEED -1; // so the value is change on every start

    /// Module
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();
    virtual void onConnectionChange();
};

#endif
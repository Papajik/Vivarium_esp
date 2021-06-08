#ifndef WATER_PUMP_H_
#define WATER_PUMP_H_

#include "../../module.h"
#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"
#include "../../../modules/internal/lcd_display/textModule.h"

#define SETTINGS_PUMP_KEY "pump"

#define FIREBASE_PUMP_CONNECTED_KEY "/pump/connected"

#define WATER_PUMP_PIN 26

class WaterPump
    : public IModule,
      public IFirebaseModule,
      public IBluetooth,
      public TextModule
{
public:
    WaterPump(int, int pin = WATER_PUMP_PIN);
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

    void startPump();
    void stopPump();

    bool isRunning();

    void setGoal(int);
    int getGoal();

    /// LCD
    std::vector<String> getText();

private:
    int _pin;
    int _levelGoal = 0;
    bool _running = false;
    bool _settingsChanged = false;
    NimBLECharacteristic *_pumpRunningCharacteristic = nullptr;

    /// Module
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();
    virtual void onConnectionChange();
};

#endif
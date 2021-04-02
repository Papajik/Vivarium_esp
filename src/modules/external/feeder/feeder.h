#ifndef _FEEDER_H
#define _FEEDER_H

#include "../../module.h"

#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"
#include <TimeAlarms.h>
#include <map>
#include <memory>

#define FEEDER_MODE_1_RPM 15
#define FEEDER_MODE_2_RPM 15
#define FEEDER_STEPS_PER_REVOLUTION 2048
#define FEEDER_IN_1 17
#define FEEDER_IN_2 16
#define FEEDER_IN_3 4
#define FEEDER_IN_4 0
#define FEEDER_MAX_TRIGERS 10

#define SETTINGS_FEEDER_KEY "feeder"

#define FIREBASE_FEEDER_CONNECTED_KEY "/feeder/connected"

#define INVALID_MEMORY_ID -1

class Stepper;

enum FeederMode
{
    SCREW = 0,
    BOX = 1,
    INVALID = -1
};

struct FeederSettings
{
    FeederMode mode;
};

struct FeedTrigger
{
    int storageId = INVALID_MEMORY_ID;
    int hour;
    int minute;
    AlarmId id;
    String firebaseKey;
    FeedTrigger() { printlnA("Feeder trigger created"); }
    ~FeedTrigger()
    {
        Serial.print("Feeder trigger ");
        Serial.print(firebaseKey);
        printlnA(" destroyed");
    }
};

struct FeedTriggerMem
{
    int hour;
    int minute;
    char key[25];
};

class Feeder : public IModule,
               public IFirebaseModule,
               public IBluetooth
{
public:
    Feeder();

    /// Firebase
    virtual void parseJson(FirebaseJson *, String);
    virtual String getSettingKey();
    virtual void parseValue(String key, String value);
    virtual void updateSensorData(FirebaseJson *);

    FeederMode getMode();
    void setMode(FeederMode);
    void feed();

    /// Bluetooth
    virtual void setupBLESettings(NimBLEService *settings);
    virtual void setupBLEState(NimBLEService *state);

    virtual void onBLEDisconnect();
    virtual void onBLEConnect();
    virtual void getHandlesCount(int *settings, int *state, int *credentials);

    void uploadTriggerToCharacteristics();
    void parseTriggerFromCharacteristics();
    void removeTriggerFromCharacteristic();

    virtual bool isFModule() { return true; }
    virtual bool isBModule() { return true; }

private:
    NimBLECharacteristic *_timeCharacteristic;
    NimBLECharacteristic *_idCharacteristic;

    bool _feeded;
    FeederSettings _settings;
    /// FirebaseKEy -> FeedTrigger
    std::map<String, std::shared_ptr<FeedTrigger>> _triggers;
    Stepper *_stepper;

    bool availableIds[FEEDER_MAX_TRIGERS];
    int asignAvailableMemoryId();

    /// Module
    virtual void onConnectionChange();
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();

    void printTrigger(std::shared_ptr<FeedTrigger>);
    void printTriggers();

    void parseTriggersJson(FirebaseJson *);
    void parseTriggerJson(FirebaseJson *, String);
    void parseTriggerValue(String, String);

    void removeTrigger(String key);

    void saveTriggerToNVS(std::shared_ptr<FeedTrigger>);
    void loadTriggersFromNVS();
    void loadTriggerFromNVS(int memoryId);

    bool _settingsChanged = false;

    bool parseTime(std::shared_ptr<FeedTrigger>, int time);
    std::shared_ptr<FeedTrigger> findTrigger(String key);
    int getTime(int hour, int minute);

    void createTrigger(int time, String key);
};

extern Feeder *feederPtr;

void feederCallback();

#endif
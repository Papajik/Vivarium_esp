#ifndef _FEEDER_H
#define _FEEDER_H

#include "../../module.h"

#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"
#include "../../../modules/internal/lcd_display/textModule.h"
#include <TimeAlarms.h>
#include <map>
#include <memory>

#include <FreeRTOS.h>
#include <freertos/semphr.h>

#define FEEDER_MODE_1_RPM 15
#define FEEDER_MODE_2_RPM 15
#define FEEDER_STEPS_PER_REVOLUTION 2048
#define FEEDER_IN_1 0
#define FEEDER_IN_2 13
#define FEEDER_IN_3 14
#define FEEDER_IN_4 27
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
};

struct FeedTriggerMem
{
    int hour;
    int minute;
    char key[25];
};

class Feeder : public IModule,
               public IFirebaseModule,
               public IBluetooth,
               public TextModule
{
public:
    Feeder(int, int in_1 = FEEDER_IN_1, int in_2 = FEEDER_IN_2, int in_3 = FEEDER_IN_3, int in_4 = FEEDER_IN_4);
    ~Feeder();

    /// Firebase
    virtual void parseJson(FirebaseJson *, String);
    virtual String getSettingKey();
    virtual void parseValue(String key, String value);
    virtual bool updateSensorData(FirebaseJson *);

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

    bool getNextTriggerTime(int *time);

    int getTriggersCount();
    int getLastFeeded();
    bool feededRecently();

    void printTriggers();

    /// LCD
    std::vector<String> getText();

private:
    SemaphoreHandle_t triggersMutex;
    void lockSemaphore(String owner);
    void unlockSemaphore();
    String _owner = "";

    void clearAllTriggers();
    std::shared_ptr<FeedTrigger> getNextTrigger();
    int _lastFeededTime = 0;

    NimBLECharacteristic *_timeCharacteristic = nullptr;
    NimBLECharacteristic *_idCharacteristic = nullptr;

    bool _feeded = false;
    FeederSettings _settings;
    /// FirebaseKEy -> FeedTrigger
    std::map<String, std::shared_ptr<FeedTrigger>> _triggers;
    std::shared_ptr<Stepper> _stepper;
    // Stepper *_stepper = nullptr;

    bool availableIds[FEEDER_MAX_TRIGERS];
    int asignAvailableMemoryId();

    /// Module
    virtual void onConnectionChange();
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();

    void printTrigger(std::shared_ptr<FeedTrigger>);

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
    void createTrigger(int time, String key);
};

extern Feeder *feederPtr;

void feederCallback();

#endif
#ifndef LED_MODULE_H
#define LED_MODULE_H

#include "../../module.h"
#include "../../../bluetooth/i_bluetooth.h"
#include "../../../firebase/i_FirebaseModule.h"
#include "../../../modules/internal/lcd_display/textModule.h"
#include <map>
#include <memory>
#include <TimeAlarms.h>

#include <FreeRTOS.h>
#include <freertos/semphr.h>

#define FIREBASE_LED_CONNECTED_KEY "/led/connected"

#define LED_PIN 25

#define SETTINGS_LED_KEY "led"

#define LED_COUNT 20
#define LED_CHANNEL 1
#define LED_TRIGGERS_MAX 10
#define INVALID_MEMORY_ID -1

class Adafruit_NeoPixel;
class Freenove_ESP32_WS2812;

struct LedTrigger
{
    int storageId = INVALID_MEMORY_ID;
    int hour;
    int minute;
    AlarmId id;
    uint32_t color;
    String firebaseKey;
};

struct LedTriggerMem
{
    int hour;
    int minute;
    uint32_t color;
    char key[25];
};

class LedModule : public IModule,
                  public IFirebaseModule,
                  public IBluetooth,
                  public TextModule
{
public:
    LedModule(int position, int pin = LED_PIN);
    ~LedModule();
    void setColor(uint32_t color);
    uint32_t getColor();

    /// Firebase
    virtual void parseJson(FirebaseJson *, String);
    virtual String getSettingKey();
    virtual void parseValue(String, String);
    virtual bool updateSensorData(FirebaseJson *);

    bool getTriggerColor(uint8_t, uint32_t *color);

    bool getNextTriggerColor(uint32_t *);
    bool getNextTriggerTime(int *);

    int getTriggersCount();

    /// LCD
    std::vector<String> getText();

    /// Bluetooth
    virtual void setupBLESettings(NimBLEService *settings);
    virtual void setupBLEState(NimBLEService *state);

    virtual void onBLEDisconnect();
    virtual void onBLEConnect();
    virtual void getHandlesCount(int *settings, int *state, int *credentials);

    void uploadTriggerToCharacteristics();
    void parseTriggerFromCharacteristics();
    void removeTriggerFromCharacteristic();

    bool isStripConnected();

private:
    SemaphoreHandle_t triggersMutex;
    void lockSemaphore(String owner);
    void unlockSemaphore();
    String _owner = "";

    bool _stripConnected = false;

    NimBLECharacteristic *_currentColorCharacteristic;

    NimBLECharacteristic *_timeCharacteristic;
    NimBLECharacteristic *_idCharacteristic;
    NimBLECharacteristic *_colorCharacteristic;

    bool availableIds[LED_TRIGGERS_MAX];

    std::shared_ptr<LedTrigger> getNextTrigger();

    int asignAvailableMemoryId();

    uint32_t _lastColor = 0;
    uint32_t _currentColor = 0;
    std::map<String, std::shared_ptr<LedTrigger>> _triggers;

    Freenove_ESP32_WS2812 *_strip;
    void showColor(bool force = false);
    void uploadColorChange();
    void printTrigger(std::shared_ptr<LedTrigger>);
    void printTriggers();

    void parseTriggersJson(FirebaseJson *);
    void parseTriggerJson(FirebaseJson *, String);
    void parseTriggerValue(String, String);

    void removeTrigger(String);

    void clearAllTriggers();

    /// Module
    virtual void onConnectionChange();
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();

    void saveTriggerToNVS(std::shared_ptr<LedTrigger>);
    void loadTriggerFromNVS(int memoryId);

    void loadTriggersFromNVS();

    bool _settingsChanged = false;

    unsigned long last = 0;

    bool parseTime(std::shared_ptr<LedTrigger>, int time);
    std::shared_ptr<LedTrigger> findTrigger(String key);
    void createTrigger(int time, int color, String key);
};

extern LedModule *ledModulePtr;
void ledTriggerCallback();

#endif
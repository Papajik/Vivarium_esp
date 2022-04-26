/**
* @file led.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef LED_MODULE_H
#define LED_MODULE_H

#include "../../module.h"
#include "../../../bluetooth/i_bluetooth.h"
#include "../../../firebase/i_FirebaseModule.h"
#include "../../../modules/internal/lcd_display/textModule.h"
#include "../../../modules/alarm/payloadAlarm.h"
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

class Freenove_ESP32_WS2812;

class LedModule : public IModule,
                  public IFirebaseModule,
                  public IBluetooth,
                  public TextModule,
                  public PayloadAlarm<uint32_t>
{
public:
    LedModule(int position, MemoryProvider *, int pin = LED_PIN);
    ~LedModule();
    void setColor(uint32_t color);
    uint32_t getColor();

    /// Firebase
    virtual void parseJson(FirebaseJson *, String);
    virtual String getSettingKey();
    virtual void parseValue(String, String);
    virtual bool updateSensorData(FirebaseJson *);

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
    virtual bool getPayloadFromJson(FirebaseJson *, uint32_t &);
    virtual bool getPayloadFromValue(String key, String value, uint32_t &);
    bool _stripConnected = false;

    NimBLECharacteristic *_currentColorCharacteristic;

    NimBLECharacteristic *_timeCharacteristic;
    NimBLECharacteristic *_idCharacteristic;
    NimBLECharacteristic *_colorCharacteristic;

    uint32_t _lastColor = 0;
    uint32_t _currentColor = 0;

    Freenove_ESP32_WS2812 *_strip;
    void showColor(bool force = false, bool triggered = true);
    void uploadColorChange();


    /// Module
    virtual void onConnectionChange();
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();

    bool _settingsChanged = false;

    unsigned long last = 0;
};

extern LedModule *ledModulePtr;


#endif
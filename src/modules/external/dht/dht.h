#ifndef _DHT11_H_
#define _DHT11_H_

#include "../../module.h"
#include "../../../firebase/i_FirebaseModule.h"
#include "../../../modules/internal/lcd_display/textModule.h"

#include "../../../bluetooth/i_bluetooth.h"

#define SETTINGS_DHT_KEY "dht"
#define SETTINGS_DHT_MIN_TEMP "dht_min_t"
#define SETTINGS_DHT_MAX_TEMP "dht_max_t"
#define SETTINGS_DHT_MIN_HUM "dht_min_h;"
#define SETTINGS_DHT_MAX_HUM "dht_max_h"
#define DHT_INVALID_VALUE -100

#define FIREBASE_DHT_CONNECTED_KEY "/dht/connected"

#define DHT_READ_DELAY 4000

#define DHT_FCM_DELAY 30 * 60 * 1000 //30 minutes

#define DHT_PIN 18

class DHT;

class DhtModule : public IModule, public IFirebaseModule, public IBluetooth, public TextModule
{
public:
    DhtModule(int, int pin = DHT_PIN);
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

    void readDht();

    float getTemp();
    float getHumidity();

    void setHumidity(float);
    void setTemp(float);

    void setMaxHum(float);
    void setMinHum(float);

    void setMaxTemp(float);
    void setMinTemp(float);

    float getMaxHum();
    float getMinHum();

    float getMaxTemp();
    float getMinTemp();
    NimBLECharacteristic *_humidityCharacteristic;

    /// LCD
    std::vector<String> getText();

private:
    float _maxTemp = 0;
    float _minTemp = 0;
    float _minHum = 0;
    float _maxHum = 0;

    unsigned long _lastFirebaseMsg = 0;

    bool _settingsChanged = false;

    DHT *dht;
    unsigned long _lastRead = 0;
    float _temp = DHT_INVALID_VALUE;
    float _humidity = DHT_INVALID_VALUE;
    NimBLECharacteristic *_tempCharacteristic;

    void checkBoundaries();

    /// Module
    virtual void onLoop();
    virtual void saveSettings();
    virtual bool loadSettings();
    virtual void onConnectionChange();
};

#endif
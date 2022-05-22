/**
 * @file wifiProvider.h
 * @author Michal Papaj (papaj.mich@gmail.com)
 * @brief
 * @version 1.0
 * @date 2021-09-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef _WIFI_PROVIDER_H_
#define _WIFI_PROVIDER_H_

/*! Importation of librairies*/
#include "../bluetooth/i_bluetooth.h"
#include "../modules/internal/lcd_display/textModule.h"
#include "../firebase/i_FirebaseModule.h"

// #include <WString.h>

/*!
 * \def WIFI_PASSWORD_KEY
 * Description
 */
#define WIFI_PASSWORD_KEY "wifi_password"
/*!
 * \def WIFI_SSID_KEY
 * Description
 */
#define WIFI_SSID_KEY "wifi_ssid"
/*!
 * \def WIFI_DELAY_KEY
 * Description
 */
#define WIFI_DELAY_KEY "wifi_delay"

/*!
 * \def CONNECTING_TIMEOUT
 * Description
 */
#define CONNECTING_TIMEOUT 10000

/*!
 * \def SSID_LENGTH
 * Description
 */
#define SSID_LENGTH 20
/*!
 * \def PASS_LENGTH
 * Description
 */
#define PASS_LENGTH 20
/*!
 * \def FIREBASE_LENGTH
 * Description
 */
#define FIREBASE_LENGTH 30

class MemoryProvider;

#define SETTINGS_WIFI_KEY "wifi"

#define INVALID_MEMORY_ID -1
#define MAX_CREDENTIALS 10
#define WIFI_MEMORY_PREFIX "wifi."

struct WifiCredentials
{
    int storageId = INVALID_MEMORY_ID;
    char pass[PASS_LENGTH];
    char ssid[SSID_LENGTH];
    char firebaseKey[FIREBASE_LENGTH];
};

class WiFiProvider : public IBluetooth, public IFirebaseModule, public TextModule
{
public:
    WiFiProvider(MemoryProvider *);
    bool setupWiFi(bool loadNVS = true);
    bool isConnected();
    int retryCredentials();
    int connect(int timetout = CONNECTING_TIMEOUT);
    int connect(std::shared_ptr<WifiCredentials> credentials, int timeout = CONNECTING_TIMEOUT);
    void disconnect();
    bool restart();
    void onLoop();

    String getPassphrase();
    void setPassphrase(String passphrase);

    String getSsid();
    void setSsid(String ssid);

    void loadFromNVS();
    void saveToNVS();

    // bluetooth
    virtual void setupBLECredentials(NimBLEService *credentials);
    virtual void onBLEDisconnect();
    virtual void onBLEConnect();
    virtual void getHandlesCount(int *settings, int *state, int *credentials);
    bool hasCredentials();

    // Firebase
    virtual void parseJson(FirebaseJson *json, String path);
    virtual String getSettingKey();
    virtual void parseValue(String key, String value);
    virtual bool updateSensorData(FirebaseJson *json) { return false; };

    virtual std::vector<String> getText();
    void printCredentials();
    void printCredentials(std::shared_ptr<WifiCredentials>);

private:
    MemoryProvider *memoryProvider;

    String _pass = "";
    String _ssid = "";
    String _connectedNetwork = "";
    bool _settingsChanged = false;

    int count = 0;
    std::map<String, std::shared_ptr<WifiCredentials>> _credentials;
    void saveCredentialsToNVS(std::shared_ptr<WifiCredentials> credentials);
    void loadAllCredentialsFromNVS();
    bool loadCredentialsFromNVS(int id);

    int getAvailableMemoryId();
    bool availableIds[MAX_CREDENTIALS];
    void freeMemoryId(int id);

    // Firebase
    void parseCredentialsJson(FirebaseJson *, String);
    void parseCredentialsJson(FirebaseJson *, bool deleteMissing = true);
    void createCredentialsFromJson(FirebaseJson *json, String credentialKey);
    void parseCredentialsValue(String key, String value);
    void removeCredentials(String);
    bool updateCredentialsFromJson(std::shared_ptr<WifiCredentials> credentials, FirebaseJson *json);
    void createNewCredentialsFromJson(FirebaseJson *json);
};

extern WiFiProvider *wifiProvider;

#endif

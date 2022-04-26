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

#include <WString.h>

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

class MemoryProvider;

class WiFiProvider : public IBluetooth, public TextModule
{
public:
    WiFiProvider(MemoryProvider *);
    bool setupWiFi();
    bool isConnected();
    int connect(int timetout = CONNECTING_TIMEOUT);
    void disconnect();
    bool restart();
    void onLoop();

    String getPassphrase();
    void setPassphrase(String passphrase);

    String getSsid();
    void setSsid(String ssid);

    void loadFromNVS();
    void saveToNVS();

    //bluetooth
    virtual void setupBLECredentials(NimBLEService *credentials);
    virtual void onBLEDisconnect();
    virtual void onBLEConnect();
    virtual void getHandlesCount(int *settings, int *state, int *credentials);
    bool hasCredentials();

    virtual std::vector<String> getText();

private:
    MemoryProvider *memoryProvider;

    String _pass = "";
    String _ssid = "";
    bool _settingsChanged = false;

    int count = 0;
};

extern WiFiProvider *wifiProvider;

#endif

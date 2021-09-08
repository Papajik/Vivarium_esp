/*!
* \file c:\Users\Papi\Documents\Arduino\Vivarium_esp\src\wifi\wifiProvider.h 
* \author Papaj Michal <papaj.mich@gmail.com>
* \version 0.1
* \date 02/04/2021
* \brief 
* \remarks None
* 
* 
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
* \def NTP_SERVER
* Description
*/
#define NTP_SERVER "pool.ntp.org"
/*!
* \def NTP_SERVER_EUROPE
* Description
*/
#define NTP_SERVER_EUROPE "europe.pool.ntp.org"
/*!
* \def NTP_SERVER_BACKUP
* Description
*/
#define NTP_SERVER_BACKUP "time.nist.gov"
/*!
* \def GMT_OFFSET_SEC
* Description
*/
#define GMT_OFFSET_SEC 3600 //+1
/*!
* \def DST_OFFSET_SEC
* Description
*/
#define DST_OFFSET_SEC 3600 //Daylight offset
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
    void setupWiFi();
    bool isConnected();
    int connect(int timetout = CONNECTING_TIMEOUT);
    void disconnect();
    void restart();
    void onLoop();

    void syncTime();

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

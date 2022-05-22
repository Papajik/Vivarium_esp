#include <WiFi.h> // ESP 32
#include <esp.h>

#include "wifiProvider.h"
#include "../memory/memory_provider.h"
#include "../utils/timeHelper.h"

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

WiFiProvider *wifiProvider;

void WiFiStationGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{

    printlnA(WiFi.localIP());

    Serial.print(ip4_addr1(&(info.got_ip.ip_info.ip)));
    Serial.print(".");
    Serial.print(ip4_addr2(&(info.got_ip.ip_info.ip)));
    Serial.print(".");
    Serial.print(ip4_addr3(&(info.got_ip.ip_info.ip)));
    Serial.print(".");
    Serial.print(ip4_addr4(&(info.got_ip.ip_info.ip)));

    printlnA();
    printlnA(IPAddress(info.got_ip.ip_info.ip.addr));
}

WiFiProvider::WiFiProvider(MemoryProvider *provider)
{
    WiFi.onEvent(WiFiStationGotIP, SYSTEM_EVENT_STA_GOT_IP);
    memoryProvider = provider;
}

bool WiFiProvider::setupWiFi(bool loadNVS)
{
    if (loadNVS)
        loadFromNVS();

    // First try credentials used to register this device
    if (connect() != WL_CONNECTED)
    {
        // Try additionalCredentials later
        return retryCredentials();
    }
    else
    {
        return WL_CONNECTED;
    }
}

int WiFiProvider::retryCredentials()
{

    if (_credentials.size() == 0)
    {
        printText({"WiFi", "No SSID"}, 3000);
        return WL_NO_SSID_AVAIL;
    }

    for (auto &&it : _credentials)
    {
        if (connect(it.second, 5000) == WL_CONNECTED)
        {
            return WL_CONNECTED;
        }
    }
    return WL_CONNECT_FAILED;
}

bool WiFiProvider::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

int WiFiProvider::connect(std::shared_ptr<WifiCredentials> credentials, int timeout)
{
    printText({"WiFi " + String(credentials->storageId), "Connecting "});
    printlnA("WiFi - connecting to backup credentials " + String(credentials->ssid));
    if (credentials->ssid[0] != '\0')
    {
        _connectedNetwork = String(credentials->ssid);
        unsigned long start = millis();
        WiFi.begin(credentials->ssid, credentials->pass);
        bool c = true;
        while (WiFi.status() != WL_CONNECTED)
        {
            printText({"WiFi " + String(credentials->storageId), c ? "Connecting." : "Connecting.."}, 1000);
            c = !c;

            printA("Status = " + WiFi.status());

            delay(500);
            if (millis() - start > timeout)
            {
                return WL_CONNECT_FAILED;
            }
        }
    }
    _connectedNetwork = "";
    return WL_NO_SSID_AVAIL;
}

int WiFiProvider::connect(int timeout)
{
    printlnA("Wifi connect - Main SSID");
    printText({"WiFi", "Connecting"});
    int status = WL_IDLE_STATUS;
    char passphrase[PASS_LENGTH];
    char ssid[SSID_LENGTH];
    _pass.toCharArray(passphrase, 30);
    _ssid.toCharArray(ssid, 30);

    if (ssid[0] != '\0')
    {
        unsigned long start = millis();
        bool connecting = true;
        status = WiFi.begin(ssid, passphrase);
        bool c = true;
        while (connecting)
        {
            printText({"WiFi", c ? "Connecting." : "Connecting.."}, 3000);
            c = !c;

            status = WiFi.status();
            if (status == WL_CONNECTED)
            {
                connecting = false;
            }
            printA("Status = ");
            printlnA(status);

            delay(500);
            if (millis() - start > timeout)
            {
                connecting = false;
                printText({"WiFi", "Timeout.."}, 3000);
                printlnE("Connection timeout");
            }
        }
    }
    else
    {
        printlnA("No ssid set");
        printText({"WiFi", "No SSID"}, 3000);
        status = WL_NO_SSID_AVAIL;
    }
    return status;
}

void WiFiProvider::disconnect()
{
    printlnI("Wifi connect");
    WiFi.disconnect();
}

bool WiFiProvider::restart()
{
    printlnI("Wifi restart");
    if (WiFi.status() == WL_CONNECTED)
    {
        disconnect();
    }
    delay(500);
    return setupWiFi();
}

String WiFiProvider::getPassphrase()
{
    return _pass;
}

void WiFiProvider::setPassphrase(String passphrase)
{
    _settingsChanged = true;
    _pass = passphrase;
}

String WiFiProvider::getSsid()
{
    return _ssid;
}

void WiFiProvider::setSsid(String ssid)
{
    _settingsChanged = true;
    _ssid = ssid;
}

void WiFiProvider::loadFromNVS()
{
    loadAllCredentialsFromNVS();
    printCredentials();
    // Use current value as default
    _pass = memoryProvider->loadString(WIFI_PASSWORD_KEY, _pass);
    _ssid = memoryProvider->loadString(WIFI_SSID_KEY, _ssid);
}

void WiFiProvider::onLoop()
{
    if (_settingsChanged)
    {
        saveToNVS();
    }
}

void WiFiProvider::saveToNVS()
{
    memoryProvider->saveString(WIFI_PASSWORD_KEY, _pass);
    memoryProvider->saveString(WIFI_SSID_KEY, _ssid);

    _settingsChanged = false;
}

bool WiFiProvider::hasCredentials()
{
    return _ssid != "" || _credentials.size() != 0;
}

std::vector<String> WiFiProvider::getText()
{
    return {};
}

void WiFiProvider::printCredentials()
{

    printlnA("**** WiFiCredentials **** ");
    printlnA("Credentials count: " + String(_credentials.size()));
    for (auto &&t : _credentials)
    {
        printA(t.first);
        printA(" >>>> ");
        printCredentials(t.second);
    }
    printlnA("***** **** **** *****");
}

void WiFiProvider::printCredentials(std::shared_ptr<WifiCredentials> credentials)
{
    debugA("ID: %d (fKey: %s) --> SSID: %s  pass: %s", credentials->storageId, credentials->firebaseKey, credentials->ssid, credentials->pass);
}

void WiFiProvider::saveCredentialsToNVS(std::shared_ptr<WifiCredentials> credentials)
{
    printlnD("saveTriggerToNVS");
    if (memoryProvider == nullptr)
        return;

    if (credentials->storageId == INVALID_MEMORY_ID)
    {
        credentials->storageId = getAvailableMemoryId();
        if (credentials->storageId == INVALID_MEMORY_ID)
        {
            printlnE("Maximum of credentials reached");
            return;
        }
    }

    debugA("Size of trigger =  %d", (int)sizeof(WifiCredentials));
    // WifiCredentials t = *credentials;
    memoryProvider->saveStruct(String(WIFI_MEMORY_PREFIX) + String(credentials->storageId), &*credentials, sizeof(WifiCredentials));
}

void WiFiProvider::loadAllCredentialsFromNVS()
{
    printlnA("Load credentials from NVS");
    if (memoryProvider != nullptr)
    {
        for (int i = 0; i < MAX_CREDENTIALS; i++)
        {
            if (loadCredentialsFromNVS(i))
            {
                availableIds[i] = false;
            }
            else
            {
                availableIds[i] = true;
            }
        }
    }
    else
    {
        printlnE("NO memory provider set. Couln't load from NVS");
    }
}

bool WiFiProvider::loadCredentialsFromNVS(int id)
{
    if (memoryProvider == nullptr)
        return false;

    // auto t = std::make_shared<WifiCredentials>();
    WifiCredentials t;
    if (memoryProvider->loadStruct(WIFI_MEMORY_PREFIX + String(id), &t, sizeof(WifiCredentials)))
    {
        _credentials.insert({t.firebaseKey, std::make_shared<WifiCredentials>(t)});
        return true;
    }
    return false;
}

int WiFiProvider::getAvailableMemoryId()
{
    for (int i = 0; i < MAX_CREDENTIALS; i++)
    {
        if (availableIds[i])
        {
            availableIds[i] = false;
            return i;
        }
    }
    return INVALID_MEMORY_ID;
}

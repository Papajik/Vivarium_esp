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

bool WiFiProvider::setupWiFi()
{
    loadFromNVS();
    return (connect() == WL_CONNECTED);
}

bool WiFiProvider::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

int WiFiProvider::connect(int timeout)
{
    printlnA("Wifi connect");
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
            if (c)
            {
                printText({"WiFi", "Connecting."}, 3000);
            }
            else
            {
                printText({"WiFi", "Connecting.."}, 3000);
            }

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
    // Use current value as default
    _pass = memoryProvider->loadString(WIFI_PASSWORD_KEY, _pass);
    _ssid = memoryProvider->loadString(WIFI_SSID_KEY, _ssid);

    printA("SSID = ");
    printlnA(_ssid.c_str());
    printA("PASS = ");
    printlnA(_pass.c_str());
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
    return _ssid != "";
}

std::vector<String> WiFiProvider::getText()
{
    return {};
}
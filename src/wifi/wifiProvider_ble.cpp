#include "wifiProvider.h"

#define CREDENTIAL_HANDLES 6

#define CHARACTERISTIC_UUID_PASSPHRASE "56600002-BE5D-4370-877B-C4A2ACE639E8"
#define CHARACTERISTIC_UUID_SSID "56600003-BE5D-4370-877B-C4A2ACE639E8"

#include <NimBLEDevice.h>
#include "../memory/memory_provider.h"
#include "../auth/auth.h"
#include "../firebase/firebase.h"

#include <WString.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

class WifiPassphraseCallbacks : public BLECharacteristicCallbacks
{
private:
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        printI("New passphrase: ");
        printlnI(pCharacteristic->getValue().c_str());
        wifiProvider.setPassphrase(pCharacteristic->getValue().c_str());
    }
};

// SSID

class WifiSsidCallbacks : public BLECharacteristicCallbacks
{
private:
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        printI("New ssid: ");
        printlnI(pCharacteristic->getValue().c_str());
        wifiProvider.setSsid(pCharacteristic->getValue().c_str());
    }
};

void WiFiProvider::setupBLECredentials(NimBLEService *credentials)
{
    printlnA("WIFI PROVIDER setupBLECredentials");
    NimBLECharacteristic *characteristicWifiPassphrase = credentials->createCharacteristic(CHARACTERISTIC_UUID_PASSPHRASE, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_AUTHEN | NIMBLE_PROPERTY::WRITE_ENC);
    NimBLECharacteristic *characteristicWifiSsid = credentials->createCharacteristic(CHARACTERISTIC_UUID_SSID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_AUTHEN | NIMBLE_PROPERTY::WRITE_ENC);
    characteristicWifiPassphrase->setCallbacks(new WifiPassphraseCallbacks());
    characteristicWifiSsid->setCallbacks(new WifiSsidCallbacks());
}

void WiFiProvider::onBLEDisconnect()
{
        restart();
}

void WiFiProvider::onBLEConnect()
{
}

void WiFiProvider::getHandlesCount(int *settings, int *state, int *credentials)
{
    *state = 0;
    *credentials = CREDENTIAL_HANDLES;
    *settings = 0;
}
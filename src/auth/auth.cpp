#include "auth.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <NimBLEDevice.h>
#include "../memory/memory_provider.h"

//*****************

// Service sensor data UUID

//*****************

#define CHARACTERISTIC_USER_ID "56600004-BE5D-4370-877B-C4A2ACE639E8"
#define CHARACTERISTIC_DEVICE_ID "56600005-BE5D-4370-877B-C4A2ACE639E8"
#define CHARACTERISTIC_IS_CLAIMED "56600006-BE5D-4370-877B-C4A2ACE639E8"
#define CHARACTERISTIC_DEVICE_TYPE "56600007-BE5D-4370-877B-C4A2ACE639E8"

#define USER_KEY "user"
#define AUTH_CREDENTIAL_HANDLES 12

Auth auth;

class UserIdCallbacks : public BLECharacteristicCallbacks
{
private:
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string s = pCharacteristic->getValue();
        if (s.at(s.size() - 1) == '*')
        {
            auth.addToUserIdBuffer(s.substr(0, s.size() - 1), true);
        }
        else
        {
            auth.addToUserIdBuffer(s, false);
        }
    }
};

class IsClaimedCallbacks : public BLECharacteristicCallbacks
{

private:
    void onRead(BLECharacteristic *pCharacteristic)
    {
        printlnA("IsClaimedCallbacks -onRead1");
        if (auth.isClaimed())
        {
            pCharacteristic->setValue("true");
        }
        else
        {
            pCharacteristic->setValue("false");
        }
    }
    
    void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, uint16_t subValue)
    {
        printlnA("Is claimed - on subscribe");
        std::string s = auth.isClaimed() ? "true" : "false";
        pCharacteristic->setValue(s);
    }
};

class DeviceIdCallbacks : public BLECharacteristicCallbacks
{

private:
    void onRead(BLECharacteristic *pCharacteristic)
    {
        printA("AUTH - getDeviceID");
        std::string s = auth.getDeviceId().c_str();
        pCharacteristic->setValue(s);
    }
};

Auth::Auth()
{
    _userId = "";
    _deviceId = "";
}

void Auth::setUserId(String id)
{
    printD("New user id = ");
    printlnD(id);

    _userId = id;
    _uidChanged = true;
    updateClaimCharacteristic();
}

void Auth::updateClaimCharacteristic()
{
    if (isBluetoothRunning())
    {
        if (isClaimed())
        {
            characteristicIsClaimed->setValue("true");
        }
        else
        {
            characteristicIsClaimed->setValue("false");
        }
        characteristicIsClaimed->notify();
    }
}

void Auth::unclaim()
{
    setUserId("");
    _saveToNVS();
}

String Auth::getUserId()
{
    return _userId;
}

bool Auth::isClaimed()
{
    return _userId != "";
}

String Auth::getDeviceId()
{
    printlnV("Get device Id.");
    printV("Returning = ");
    printlnV(_deviceId);
    return _deviceId;
}

void Auth::setDeviceId(String deviceId)
{
    printV("Saving deviceID: ");
    printlnV(deviceId);
    _deviceId = deviceId;
}

void Auth::setupBLECredentials(BLEService *credentials)
{

    printlnA("Auth setup BLECredentials");

    /// USER ID
    BLECharacteristic *characteristicUserId = credentials->createCharacteristic(CHARACTERISTIC_USER_ID,
                                                                                NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_AUTHEN | NIMBLE_PROPERTY::WRITE_ENC);
    characteristicUserId->setCallbacks(new UserIdCallbacks());

    /// DEVICE ID
    deviceIdCharacteristic = credentials->createCharacteristic(CHARACTERISTIC_DEVICE_ID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_AUTHEN | NIMBLE_PROPERTY::READ_ENC);
    deviceIdCharacteristic->setCallbacks(new DeviceIdCallbacks());

    /// IS CLAIMED
    characteristicIsClaimed = credentials->createCharacteristic(CHARACTERISTIC_IS_CLAIMED,
                                                                NIMBLE_PROPERTY::READ |
                                                                    NIMBLE_PROPERTY::READ_AUTHEN |
                                                                    NIMBLE_PROPERTY::READ_ENC |
                                                                    NIMBLE_PROPERTY::NOTIFY |
                                                                    NIMBLE_PROPERTY::INDICATE);
    characteristicIsClaimed->setCallbacks(new IsClaimedCallbacks());
}

void Auth::onBLEDisconnect()
{
    _saveToNVS();
}

void Auth::onBLEConnect()
{
    printA("AUTH - device ID = ");
    printlnA(_deviceId);
    std::string s = _deviceId.c_str();
    deviceIdCharacteristic->setValue(s);
    s = isClaimed() ? "true" : "false";
    printA("Claimed = ");
    printlnA(s.c_str());
    characteristicIsClaimed->setValue(s);
}

void Auth::getHandlesCount(int *settings, int *state, int *credentials)
{
    *settings = 0;
    *state = 0;
    *credentials = AUTH_CREDENTIAL_HANDLES;
}

void Auth::loadFromNVS()
{
    _userId = memoryProvider.loadString(USER_KEY, "");
    printlnA("Load from NVS");
    printA("USER ID = ");
    printlnA(_userId);
}

void Auth::_saveToNVS()
{
    printlnD("Auth - onBLEDisconnect");
    if (_uidChanged)
    {
        printD("Saving user id = ");
        printlnD(_userId);
        _uidChanged = false;
        memoryProvider.saveString(USER_KEY, _userId);
    }
}

void Auth::addToUserIdBuffer(std::string s, bool isFinal)
{
    printD("Adding to buffer ");
    printD(s.c_str());
    printD(" (");
    printD(s.size());
    printlnD(")");
    _userIdbuffer.append(s);
    if (isFinal)
    {
        setUserId(String(_userIdbuffer.c_str()));
        clearUserIdBuffer();
    }
}

void Auth::clearUserIdBuffer()
{
    _userIdbuffer = "";
}
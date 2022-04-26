/**
* @file auth.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef _AUTH_H_
#define _AUTH_H_

#include <HardwareSerial.h>
#include "../bluetooth/i_bluetooth.h"

#include <string>

class NimBLECharacteristic;
class MemoryProvider;

class Auth : public IBluetooth
{
public:
    Auth(MemoryProvider *);
    void setUserId(String id);
    String getUserId();

    bool isClaimed();
    void unclaim();
    void updateClaimCharacteristic();
    void addToUserIdBuffer(std::string, bool isFinal);
    void clearUserIdBuffer();
    String getDeviceId();
    void setDeviceId(String deviceId);

    void loadFromNVS();
    void _saveToNVS();

    virtual void setupBLECredentials(NimBLEService *credentials);
    virtual void onBLEDisconnect();
    virtual void onBLEConnect();
    virtual void getHandlesCount(int *settings, int *state, int *credentials);

private:
    String _userId = "";
    String _deviceId = "";
    bool _uidChanged = false;
    NimBLECharacteristic *deviceIdCharacteristic = nullptr;
    NimBLECharacteristic *characteristicIsClaimed = nullptr;
    std::string _userIdbuffer;
    MemoryProvider *_memoryProvider = nullptr;
};

#endif
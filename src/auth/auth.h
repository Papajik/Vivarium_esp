#ifndef _AUTH_H_
#define _AUTH_H_

#include <HardwareSerial.h>
#include "../bluetooth/i_bluetooth.h"

#include <string>

class NimBLECharacteristic;

class Auth : public IBluetooth
{
private:
    String _userId;

public:
    Auth();
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
    String _deviceId;
    bool _uidChanged;
    NimBLECharacteristic *deviceIdCharacteristic;
    NimBLECharacteristic *characteristicIsClaimed;
    std::string _userIdbuffer;
};

extern Auth auth;

#endif
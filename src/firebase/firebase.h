#ifndef _FIREBASE_SERVICE_H_
#define _FIREBASE_SERVICE_H_

#include <Arduino.h>

#include "../bluetooth/i_bluetooth.h"

#define NUMBER_OF_PATHS 5

#include <vector>
#include <map>

class FirebaseData;
class FirebaseJson;
class FirebaseStream;

class IFirebaseModule;
class MultiPathStream;
class MemoryProvider;
class Auth;
class MessagingService;

enum ChildPath
{
    SETTINGS = 0,
    STATE = 1,
    FIRMWARE = 2,
    ACTIVE_STATUS = 3,
    BLE_NAME = 4
};

class FirebaseService : public IBluetooth
{
public:
    FirebaseService(Auth *, MemoryProvider *, MessagingService *);
    ~FirebaseService();

    void setupFirebase();
    void stopFirebase();
    void startFirebase();

    void checkStop();

    void setStartInFuture();
    void setStopInFuture();

    bool isRunning();
    void setRunning(bool);
    void uploadSensorData();
    void logNewStart();
    void checkActiveStatus();
    void jsonCallback(FirebaseJson *, String);
    void valueCallback(MultiPathStream *);
    void addModule(IFirebaseModule *m);

    void checkSSLConnected();

    void factoryReset();
    void clearVersion();

    void onLoop();

    // Messaging

    void uploadState(std::string, bool);
    void uploadState(std::string, float);
    void uploadState(std::string, int);

    void uploadCustomData(std::string, std::string, std::string);
    void uploadCustomData(std::string, std::string, float);

    String childPaths[NUMBER_OF_PATHS] = {"/settings", "/state", "/info/firmware", "/info/active", "/info/name"};

    String getFirmwareName(String);

    String getFirmwareDownloadUrl(String);

    // int checkFirebase();

    virtual void onBLEDisconnect();
    virtual void onBLEConnect();
    virtual void getHandlesCount(int *settings, int *state, int *credentials);

private:
    Auth *_auth;
    MemoryProvider *_memoryProvider;
    MessagingService *_messagingService;

    bool _toStart = false;
    bool _toStop = false;

    unsigned long _lastTokenRefresh = 0;
    unsigned long _lastUploadTime = 0;

    void startStream();
    void stopStream();

    std::vector<IFirebaseModule *> _modules;

    bool _running = false;
    bool _initialized = false;
};

extern FirebaseService *firebaseService;

#endif
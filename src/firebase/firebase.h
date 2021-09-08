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

    void uploadState(String, bool);
    void uploadState(String, float);
    void uploadState(String, int);

    void uploadCustomData(String, String, String);
    void uploadCustomData(String, String, float);

    String childPaths[NUMBER_OF_PATHS] = {"/settings", "/state", "/info/firmware", "/info/active", "/info/name"};

    String getFirmwareName(String);

    String getFirmwareDownloadUrl(String);

    int checkFirebase();

    virtual void onBLEDisconnect();
    virtual void onBLEConnect();
    virtual void getHandlesCount(int *settings, int *state, int *credentials);

private:
    Auth *_auth;
    MemoryProvider *_memoryProvider;
    MessagingService *_messagingService;

    bool _toStart = false;
    bool _toStop = false;

    int _errorCount = 0;

    unsigned long _lastTokenRefresh = 0;
    unsigned long _lastUploadTime = 0;
    unsigned long _lastCleanTime = 0; // clear firebaseObject every 2:50 to provide enough heap for SSL hanshake
    unsigned long _lastFCMTokenCheck = 0;


    void startStream();
    void stopStream();

    int count = 2541;
    std::vector<IFirebaseModule *> _modules;

    bool _running = false;
    bool _initialized = false;
};

extern FirebaseService *firebaseService;

#endif
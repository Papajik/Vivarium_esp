#ifndef _FIREBASE_SERVICE_H_
#define _FIREBASE_SERVICE_H_

#include <Arduino.h>
#include <freertos/semphr.h>

#define STORAGE_BUCKET_ID "vivarium-control-unit.appspot.com"
#define FIREBASE_HOST "vivarium-control-unit.firebaseio.com"
#define FIREBASE_TOKEN "bd2c2a9af5f31269ce8868e8b0051839"
#define FIREBASE_CERT_FILE "/cert.cer"
#include "../bluetooth/i_bluetooth.h"
#define FIREBASE_SERVICE_ACCOUNT "/service_account_file.json"
#define DEFAULT_UPLOAD_DELAY 60000                 //1 min
#define REFRESH_FCM_TOKENS_INTERVAL 60 * 60 * 1000 // every 1 hour refresh FCM tokens

#define FBDO_CLEAR_DELAY 170 * 1000

#define NUMBER_OF_PATHS 5

#include <vector>
#include <map>

class FirebaseData;
class FirebaseJson;
class FirebaseStream;
struct fb_esp_auth_signin_provider_t;
struct fb_esp_cfg_t;
class IFirebaseModule;
class MultiPathStream;

// enum FCM_TYPE
// {
//     CONNECTION,
//     VALUE
// };

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
    FirebaseService();

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

    void onLoop();



    // Messaging
    void refreshFCMTokens();
    void getFCMSettings();
    void parseFCMTokens(FirebaseJson *);

    void sendFCM(String title, String body, String token, bool timePrefix);

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
    bool _toStart = false;
    bool _toStop = false;

    unsigned long _lastValidUpdate = 0;

    unsigned long _lastTokenRefresh = 0;
    unsigned long _lastUploadTime = 0;
    unsigned long _lastCleanTime = 0; // clear firebaseObject every 2:50 to provide enough heap for SSL hanshake
    unsigned long _lastFCMTokenCheck = 0;

    int _delayFCMNotification = 0;
    bool _notificationConnectionOn = false;
    bool _notificationCrossLimit = false;

    std::map<String, unsigned long> _lastValueSendTimeMap;

    String lockOwner = "";
    void lockSemaphore(String log);
    void unlockSemaphore();

    std::vector<String>
        _firebaseMessagingTokens;

    SemaphoreHandle_t bdoMutex;
    void startStream();
    void stopStream();

    int count = 2541;
    std::vector<IFirebaseModule *> _modules;

    bool _running = false;
    bool _initialized = false;
    FirebaseData *firebaseBdo;
    FirebaseData *firebaseStreamBdo;

    fb_esp_auth_signin_provider_t *firebaseAuth;
    fb_esp_cfg_t *firebaseConfig;
};

extern FirebaseService firebaseService;

#endif
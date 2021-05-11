#include "firebase.h"

#include "service_account.h"
#include "cert.h"
#include <HardwareSerial.h>
#include "../auth/auth.h"
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <Firebase_ESP_Client.h>
#include "i_FirebaseModule.h"
#include "../ota/ota.h"
#include "../wifi/wifiProvider.h"
#include "../debug/memory.h"
#include <Esp.h>
#include "../memory/memory_provider.h"
#include "messagingService.h"
#include "../bluetooth/bluetooth.h"

#include "firebaseBdo.h"

#include "semaphore/firebaseSemaphore.h"

// #define FIREBASE_TOKEN "bd2c2a9af5f31269ce8868e8b0051839"
// #define FIREBASE_SERVICE_ACCOUNT "/service_account_file.json"

#define STORAGE_BUCKET_ID "vivarium-control-unit.appspot.com"
#define FIREBASE_HOST "vivarium-control-unit.firebaseio.com"
#define FIREBASE_CERT_FILE "/cert.cer"

#define DEFAULT_UPLOAD_DELAY 60000                 //1 min
#define REFRESH_FCM_TOKENS_INTERVAL 60 * 60 * 1000 // every 1 hour refresh FCM tokens
#define FBDO_CLEAR_DELAY 170 * 1000

FirebaseService *firebaseService;

// void printResult(FirebaseStream &data);
void streamCallback(MultiPathStream data);
void streamTimeoutCallback(bool timeout);

FirebaseService::FirebaseService(Auth *auth, MemoryProvider *provider, MessagingService *service)
{
    firebaseAuth = new FirebaseAuth();
    firebaseConfig = new FirebaseConfig();
    firebaseStreamBdo = new FirebaseData();
    _modules.reserve(8);
    _firebaseMessagingTokens.reserve(5);

    _auth = auth;
    _memoryProvider = provider;

    _running = false;
    _initialized = false;
}

void FirebaseService::checkStop()
{
    if (_toStop)
    {
        _toStop = false;
        stopFirebase();
    }
}

void FirebaseService::setStartInFuture()
{
    _toStart = true;
}

void FirebaseService::setStopInFuture()
{
    _toStop = true;
}

void FirebaseService::setupFirebase()
{
    if (!_initialized)
    {
        // firebaseConfig->cert.file = FIREBASE_CERT_FILE;
        // firebaseConfig->cert.file_storage = mem_storage_type_flash;

        firebaseConfig->cert.data = cert;
        firebaseConfig->host = FIREBASE_HOST;

        firebaseConfig->service_account.data.client_email = "firebase-adminsdk-dg0j0@vivarium-control-unit.iam.gserviceaccount.com";
        firebaseConfig->service_account.data.client_id = "115674499447632950713";
        firebaseConfig->service_account.data.private_key = private_key;
        firebaseConfig->service_account.data.private_key_id = "66837c5ae5c45ba17275a615f27382cc1eaac9ef";
        firebaseConfig->service_account.data.project_id = "vivarium-control-unit";
        firebaseAuth->token.uid = ""; //Empty for sign in as admin

        Firebase.begin(firebaseConfig, firebaseAuth);
        Firebase.reconnectWiFi(true);
        printlnA("Firebase begin");
        firebaseStreamBdo->setResponseSize(2048);
        firebaseBdo->setResponseSize(1024);

        Firebase.setFloatDigits(2);
        Firebase.setDoubleDigits(2);

        printlnA("paths: ");
        for (int i = 0; i < NUMBER_OF_PATHS; i++)
        {
            printlnA(childPaths[i].c_str());
        }

        _initialized = true;
    }
    checkActiveStatus();
    getFCMSettings();
    startFirebase();
}

void FirebaseService::checkActiveStatus()
{
    printlnA("checkActiveStatus");
    String path = "/devices/" + _auth->getDeviceId() + "/info/active";
    firebaseSemaphore.lockSemaphore("checkActiveStatus");
    if (Firebase.RTDB.getBool(firebaseBdo, path.c_str()))
    {
        printlnA("Active status recieved");
        printA("Active = ");
        printlnA(firebaseBdo->boolData() ? "true" : "false");
        printlnA(firebaseBdo->dataPath());
        if (!firebaseBdo->boolData())
        {
            factoryReset();
        }
    }
    else
    {

        printlnA("Couldnt receive active status");
    }
    firebaseSemaphore.unlockSemaphore();
}

void FirebaseService::stopStream()
{
    printlnA("STOPPING STREAM");
    printMemory();
    firebaseStreamBdo->clear();
    Firebase.RTDB.endStream(firebaseStreamBdo);

    printMemory();
    printlnV("STREAM STOPPED");
}

void FirebaseService::startStream()
{
    printlnA("STARTING STREAM");
    printMemory();
    std::string s = "/devices/";
    s.append(_auth->getDeviceId().c_str());
    if (!Firebase.RTDB.beginMultiPathStream(firebaseStreamBdo, s.c_str(), childPaths, NUMBER_OF_PATHS))
    {
        printlnA("------------------------------------");
        printlnA("Can't begin stream connection...");
        printlnA("REASON: " + firebaseStreamBdo->errorReason());
        printlnA("------------------------------------");
        printlnA();
    }
    Firebase.RTDB.setMultiPathStreamCallback(firebaseStreamBdo, streamCallback, streamTimeoutCallback);
    printlnA("STREAM STARTED");
    printMemory();
}

void FirebaseService::onLoop()
{
    if (_toStart && wifiProvider->isConnected())
    {
        _toStart = false;
        startFirebase();
    }

    if (_running)
    {

        // Clear bdo object to clear heap memory for SSL hanshake
        if (millis() - _lastCleanTime > FBDO_CLEAR_DELAY)
        {
            _lastCleanTime = millis();
            firebaseSemaphore.lockSemaphore("onLoop");
            if (firebaseBdo->httpConnected())
            {

                printlnA("Clearing object");
                firebaseBdo->clear();
            }
            firebaseSemaphore.unlockSemaphore();
        }

        if (millis() - _lastUploadTime > DEFAULT_UPLOAD_DELAY)
        {
            if (wifiProvider->isConnected())
            {
                uploadSensorData();
            }

            _lastUploadTime = millis();
        }

        if (millis() - _lastTokenRefresh > REFRESH_FCM_TOKENS_INTERVAL)
        {
            if (wifiProvider->isConnected())
            {
                refreshFCMTokens();
            }
            _lastTokenRefresh = millis();
        }
    }
}

void FirebaseService::addModule(IFirebaseModule *m)
{
    _modules.push_back(m);
}

bool FirebaseService::isRunning() { return _running; }

void FirebaseService::setRunning(bool r)
{
    printlnA("FB: set running = " + r ? "true" : "false");
    _lastValidUpdate = millis();
    _running = r;
}

void FirebaseService::uploadSensorData()
{
    if (_running)
    {
        time_t now;
        time(&now);

        char time[40];
        ultoa(now, time, 10);

        String Path = String("/sensorData/") + _auth->getDeviceId() + String("/") + String(time);

        FirebaseJson json;

        for (IFirebaseModule *m : _modules)
        {
            m->updateSensorData(&json);
        }
        String buffer;
        json.toString(buffer, true);
        printlnV("Uploading sensor data");
        printlnV(buffer);

        firebaseSemaphore.lockSemaphore("uploadSensorData");
        if (Firebase.RTDB.set(firebaseBdo, Path.c_str(), &json))
        {
            _lastValidUpdate = millis();
            printlnD("Sensor data uploaded");
        }
        else
        {
            printlnE("Sensor data upload FAILED");
            printlnE(firebaseBdo->errorReason());
        }
        firebaseSemaphore.unlockSemaphore();
    }
}

void FirebaseService::onBLEDisconnect()
{
    if (wifiProvider->isConnected())
    {
        setupFirebase();
    }
}
void FirebaseService::onBLEConnect()
{
}

void FirebaseService::getHandlesCount(int *settings, int *state, int *credentials) {}

void FirebaseService::logNewStart()
{
    if (!_running)
    {
        printlnA("Firebase is not running ");
        return;
    }
    printlnA("Log new start");

    time_t now;
    time(&now);
    char time[40];
    ultoa(now, time, 10);

    String Path = "/start/" + _auth->getDeviceId() + "/" + String(time);

    firebaseSemaphore.lockSemaphore("logNewStart");
    if (Firebase.RTDB.set(firebaseBdo, Path.c_str(), true))
    {

        printlnA("PASSED");
        printlnA("PATH: " + firebaseBdo->dataPath());
        printlnA("TYPE: " + firebaseBdo->dataType());
        printlnA("ETag: " + firebaseBdo->ETag());
        printlnA("------------------------------------");
        printlnA();
        _lastValidUpdate = millis();
    }
    else
    {
        printlnA("FAILED");
        printlnA("REASON: " + firebaseBdo->errorReason());
        printlnA("------------------------------------");
        printlnA();
    }
    firebaseSemaphore.unlockSemaphore();
}

void FirebaseService::factoryReset()
{
    _memoryProvider->factoryReset();
    ESP.restart();
}

void FirebaseService::jsonCallback(FirebaseJson *json, String path)
{
    printlnD("JSON Callback");
    String buffer;
    json->toString(buffer, true);
    printlnD(buffer);
    for (IFirebaseModule *module : _modules)
    {
        module->parseJson(json, path);
    }
}
void FirebaseService::valueCallback(MultiPathStream *data)
{
    printlnI("Firebase valueCallback");
    printMemory();
    printlnI("FB Parse value");
    printlnI("Data path = " + data->dataPath);
    printlnI("Data type = " + data->type);

    String dataPath = data->dataPath.substring(1);

    int index = dataPath.indexOf("/");
    int secondIndex = dataPath.indexOf("/", index + 1);

    String settingsKey = dataPath.substring(index + 1, secondIndex);
    printlnI("key = " + settingsKey);
    printI("Modules ");
    printlnI((int)_modules.size());

    for (IFirebaseModule *module : _modules)
    {
        String s = module->getSettingKey();

        printI("Trying module");
        printlnI(s);

        if (s == settingsKey)
        {
            printlnI("Settings key equals");
            module->parseValue(data->dataPath, data->value);
            printMemory();
        }
        else
        {
            printlnI("settings key is not equal");
        }
    }
}

void FirebaseService::uploadState(String key, bool value)
{
    if (_running)
    {

        printlnI("uploading state");

        String path = String("devices/") + _auth->getDeviceId() + String("/state") + key;
        checkSSLConnected();
        firebaseSemaphore.lockSemaphore("upload state");
        if (Firebase.RTDB.set(firebaseBdo, path.c_str(), value))
        {
            printlnD("State uploaded");
            _lastValidUpdate = millis();
        }
        else
        {

            printlnE("State upload FAILED");
            printlnE(firebaseBdo->errorReason());
        }
        firebaseSemaphore.unlockSemaphore();
    }
}

void FirebaseService::uploadState(String key, float value)
{
    if (_running)
    {
        printlnA("FB: Uploading state: " + key);
        String path = String("devices/") + _auth->getDeviceId() + String("/state") + key;
        firebaseSemaphore.lockSemaphore("uploadState");
        if (Firebase.RTDB.set(firebaseBdo, path.c_str(), value))
        {
            printlnV("State uploaded");
            _lastValidUpdate = millis();
        }
        else
        {

            printlnE("State upload FAILED");
            printlnE(firebaseBdo->errorReason());
        }
        firebaseSemaphore.unlockSemaphore();
    }
}
void FirebaseService::uploadState(String key, int value)
{
    if (_running)
    {
        firebaseSemaphore.lockSemaphore("uploadState");
        printlnA("FB: Uploading state: " + key);
        String path = String("devices/") + _auth->getDeviceId() + String("/state") + key;
        if (Firebase.RTDB.set(firebaseBdo, path.c_str(), value))
        {
            printlnV("State uploaded");
            _lastValidUpdate = millis();
        }
        else
        {
            printlnE(firebaseBdo->errorReason());
            printlnE("State upload FAILED");
        }
        firebaseSemaphore.unlockSemaphore();
    }
}

void FirebaseService::stopFirebase()
{

    printlnA("Stopping firebase");
    setRunning(false);
    firebaseSemaphore.lockSemaphore("stopFirebase");
    stopStream();

    firebaseBdo->clear();
    firebaseSemaphore.unlockSemaphore();

    printlnV("Firebase stopped");
}

void FirebaseService::startFirebase()
{
    if (!isRunning())
    {
        printlnA("Starting firebase");
        startStream();
        setRunning(true);
        printMemory();
        printlnA("Firebase started");
    }
    else
    {
        printlnA("Firebase already running");
    }
}

String FirebaseService::getFirmwareName(String version)
{
    if (!_running)
        return "";
    String path = String("firmware/" + version + "/filename");
    firebaseSemaphore.lockSemaphore("getFirmwareName");
    if (Firebase.RTDB.getString(firebaseBdo, path.c_str()))
    {
        printlnA("Version = " + version);
        printlnA("Firmware name = " + firebaseBdo->stringData());
        firebaseSemaphore.unlockSemaphore();
        return firebaseBdo->stringData();
    }
    else
    {
        printlnA("Version not found");
        firebaseSemaphore.unlockSemaphore();
        return "";
    }
}

/**
 * @brief Get dowload URL for given firmware version
 * 
 * @param version Version of firmware 
 * @return String Download URL of file, empty URL when no download link is no available
 */
String FirebaseService::getFirmwareDownloadUrl(String version)
{

    String fileName = getFirmwareName(version);
    if (fileName == "")
        return "";
    String filePath = "firmware/" + fileName;
    String url;
    firebaseSemaphore.lockSemaphore("getFirmwareDownloadUrl");

    if (Firebase.Storage.getMetadata(firebaseBdo, STORAGE_BUCKET_ID, filePath.c_str()))
    {
        url = firebaseBdo->downloadURL();
    }
    else
    {
        url = "";
    }

    firebaseSemaphore.unlockSemaphore();
    return url;
}

void FirebaseService::uploadCustomData(String prefix, String suffix, String data)
{
    if (_running)
    {

        printlnV("uploadCustomData");
        String path = prefix + _auth->getDeviceId() + suffix;

        checkSSLConnected();
        firebaseSemaphore.lockSemaphore("uploadCustomData");
        if (Firebase.RTDB.set(firebaseBdo, path.c_str(), data))
        {
            printlnV("custom data uploaded");
            _lastValidUpdate = millis();
        }
        else
        {
            printlnE("custom data upload FAILED");
            printlnE(firebaseBdo->errorReason());
        }
        firebaseSemaphore.unlockSemaphore();
    }
}

int FirebaseService::checkFirebase()
{
    if (_running)
    {
        //Fail safe - resets ESP when firebase token handshake hangs for too long
        if (millis() - _lastValidUpdate > DEFAULT_UPLOAD_DELAY * 6)
        {
            printlnE("FIREABSE ERROR");
            printlnA("Firebase ERROR - RESTARTING");

            firebaseSemaphore.lockSemaphore("checkFirebase");
            firebaseBdo->pauseFirebase(true);
            firebaseSemaphore.unlockSemaphore();
            return -1;
        }
    }
    return 0;
}

void FirebaseService::uploadCustomData(String prefix, String suffix, float data)
{
    if (_running)
    {
        printlnV("uploadCustomData");
        String path = prefix + _auth->getDeviceId() + suffix;

        checkSSLConnected();

        firebaseSemaphore.lockSemaphore("uploadCustomData");
        if (Firebase.RTDB.set(firebaseBdo, path.c_str(), data))
        {
            printlnV("custom data uploaded");
            _lastValidUpdate = millis();
        }
        else
        {
            printlnE(firebaseBdo->errorReason());
            printlnE("custom data upload FAILED");
        }
        firebaseSemaphore.unlockSemaphore();
    }
}

void streamCallback(MultiPathStream data)
{
    printlnA("Stream callback");

    // Check active status
    data.get(firebaseService->childPaths[ChildPath::ACTIVE_STATUS]);
    if (data.type == "boolean" && data.value == "false")
    {
        firebaseService->factoryReset();
    }

    //Check firmware
    data.get(firebaseService->childPaths[ChildPath::FIRMWARE]);
    if (data.type == "string" && (data.value != ""))
    {
        if (otaService != nullptr)
        {
            if (otaService->isNewVersion(data.value))
            {
                String url = firebaseService->getFirmwareDownloadUrl(data.value);
                if (!url.isEmpty())
                {
                    firebaseService->stopFirebase();
                    bleController->stop();
                    if (!otaService->prepareAndStartUpdate(url, data.value))
                    {
                        //If preparation failed, start firebase again
                        firebaseService->startFirebase();
                    }
                }
            }
        }
    }

    // Check Bluetooth name
    data.get(firebaseService->childPaths[ChildPath::BLE_NAME]);
    if (data.type == "string" && (data.value != ""))
    {
        bleController->setBleName(data.value);
    }

    for (int i = 0; i < 2; i++)
    {
        data.get(firebaseService->childPaths[i]);
        printlnA(data.dataPath);
        if (data.type == "json")
        {
            FirebaseJson json;
            json.setJsonData(data.value);
            firebaseService->jsonCallback(&json, data.dataPath);
        }
        else if (data.type != "")
        {
            firebaseService->valueCallback(&data);
        }
    }
}

void streamTimeoutCallback(bool timeout)
{
    if (timeout)
    {
        printlnW("FB: WARN - TIMEOUT");
    }
}

void FirebaseService::checkSSLConnected()
{
    firebaseSemaphore.lockSemaphore("checkSSLConnected");
    if (!firebaseBdo->httpConnected())
    {
        printlnA("Clearing firebase connection");
        firebaseBdo->clear();
    }
    firebaseSemaphore.unlockSemaphore();
}

void FirebaseService::refreshFCMTokens()
{

    String path = "/users/" + _auth->getUserId() + "/ tokens";
    printlnA(path);
    firebaseSemaphore.lockSemaphore("refreshFCMTokens");
    if (Firebase.RTDB.getJSON(firebaseBdo, path.c_str()))
    {
        printlnA("FCM tokens recieved");
        printlnA(firebaseBdo->dataType());

        FirebaseJson &json = firebaseBdo->jsonObject();

        parseFCMTokens(&json);
    }
    else
    {
        printlnA("Couldnt receive tokens");
    }
    firebaseSemaphore.unlockSemaphore();
}

void FirebaseService::parseFCMTokens(FirebaseJson *json)
{
    _firebaseMessagingTokens.clear();

    printlnA("Pretty printed JSON data:");
    printlnA();
    printlnA("Iterate JSON data:");
    printlnA();
    size_t len = json->iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
        json->iteratorGet(i, type, key, value);
        printA(", Key: ");
        printA(key);
        printA(", Value: ");
        printlnA(value);
        if (value == "true")
        { //If key is valid then add token
            // _firebaseMessagingTokens.push_back(key);
            _messagingService->addToken(key);
        }
    }
    json->iteratorEnd();
}

void FirebaseService::getFCMSettings()
{
    printlnA("Get FCM Settings");
    String path = "/users/" + _auth->getUserId() + "/settings";
    firebaseSemaphore.lockSemaphore("getFCMSettings");
    if (Firebase.RTDB.getJSON(firebaseBdo, path.c_str()))
    {
        printlnA("FCM Settings recieved");
        printlnA(firebaseBdo->dataType());
        printlnA(firebaseBdo->jsonString());

        _firebaseMessagingTokens.clear();
        FirebaseJson &json = firebaseBdo->jsonObject();
        FirebaseJsonData data;

        if (json.get(data, "/nDelay", false))
        {
            // _delayFCMNotification = data.intValue;
            _messagingService->setDelayFCM(data.intValue);
        }

        if (json.get(data, "/nLimit", false))
        {
            // _notificationCrossLimit = data.boolValue;
            _messagingService->setNotifyOnCrossLimit(data.boolValue);
        }

        if (json.get(data, "/nConn", false))
        {
            // _notificationConnectionOn = data.boolValue;
            _messagingService->setNotifyOnConnectionChange(data.boolValue);
        }

        if (json.get(data, "/nDist", false))
        {
            _messagingService->setDistinctNotification(data.boolValue);
        }

        /// Refresh tokens
        if (json.get(data, "/tokens", false))
        {
            FirebaseJson tokenJson;
            if (data.getJSON(tokenJson))
            {
                parseFCMTokens(&tokenJson);
            }
        }

        printA("DELAY = ");
        printlnA(_delayFCMNotification);
        printA("limit = ");
        printlnA(_notificationCrossLimit);
        printA("connection = ");
        printlnA(_notificationConnectionOn);
    }
    else
    {

        printlnA("Couldnt receive settings");
    }
    firebaseSemaphore.unlockSemaphore();
}

#include "firebase.h"

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

#define API_KEY "AIzaSyCrVwfunuauskErYhbScfK4rXh2RTG4v0o"

#define FIREBASE_PROJECT_ID "vivarium-control-unit"
#define FIREBASE_SERVICE_EMAIL "firebase-adminsdk-dg0j0@vivarium-control-unit.iam.gserviceaccount.com"

#define STORAGE_BUCKET_ID "vivarium-control-unit.appspot.com"
#define DATABASE_URL "vivarium-control-unit.firebaseio.com"

#define USER_EMAIL "device@fw.com"
#define USER_PASSWORD "AZE}(vYJxe}Z'3rU"

#define DEFAULT_UPLOAD_DELAY 60000                 //1 min
#define REFRESH_FCM_TOKENS_INTERVAL 60 * 60 * 1000 // every 1 hour refresh FCM tokens
#define FBDO_CLEAR_DELAY 170 * 1000
#define FIREBASE_READY_DELAY 60000

FirebaseService *firebaseService;
FirebaseData *firebaseStreamBdo;

// void printResult(FirebaseStream &data);
void streamCallback(MultiPathStream data);
void streamTimeoutCallback(bool timeout);

FirebaseAuth firebaseAuth;
FirebaseConfig firebaseConfig;

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

bool _initializeFB(bool serviceAccount = true)
{
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    firebaseConfig.api_key = API_KEY;
    firebaseConfig.time_zone = 2;
    if (serviceAccount)
    {
        firebaseConfig.service_account.data.client_email = FIREBASE_SERVICE_EMAIL;
        firebaseConfig.service_account.data.project_id = FIREBASE_PROJECT_ID;
        firebaseConfig.service_account.data.private_key = service_account_key;
    }
    else
    {
        firebaseAuth.user.email = USER_EMAIL;
        firebaseAuth.user.password = USER_PASSWORD;
    }

    firebaseConfig.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    firebaseConfig.database_url = DATABASE_URL;
    firebaseConfig.max_token_generation_retry = 5;
    // firebaseConfig.cert.data = cert;

    Serial.println("Firebase begin");
    Firebase.begin(&firebaseConfig, &firebaseAuth);
    
    Serial.println("Firebase begin done");
    Firebase.reconnectWiFi(true);

    unsigned long start = millis();
    Serial.println("Checking ready");
    while (!Firebase.ready())
    {
        delay(500);
        Serial.println("Firebase.ready()?");
        if (millis() - start > 10000)
        {
            Serial.println("Token init failed");
            return false;
        }
    }
    Serial.println("Token OK");
    return true;
}

FirebaseService::FirebaseService(Auth *auth, MemoryProvider *provider, MessagingService *service) : _auth(auth), _memoryProvider(provider), _messagingService(service)
{
    firebaseStreamBdo = new FirebaseData();
    firebaseStreamBdo->setCert(rtdb_cert);
    firebaseBdo = new FirebaseData();
    firebaseBdo->setCert(rtdb_cert);
    _modules.reserve(8);
}

FirebaseService::~FirebaseService()
{
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
    printlnA("Setup FIREBASE");
    if (!_initialized)
    {
        _initialized = _initializeFB();
    }

    if (!_initialized)
            return;

    checkActiveStatus();
    if (_messagingService != nullptr)
        _messagingService->init();
    startFirebase();
}

void FirebaseService::checkActiveStatus()
{
    printlnA("checkActiveStatus");
    String path = "/devices/" + _auth->getDeviceId() + "/info/active";
    printlnA(path);
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
        printlnA(firebaseBdo->errorReason());
    }
    firebaseSemaphore.unlockSemaphore();
}

void FirebaseService::stopStream()
{
    printlnA("STOPPING STREAM");
    printMemory();
    firebaseStreamBdo->clear();
    Firebase.RTDB.endStream(firebaseStreamBdo);
    firebaseStreamBdo->clear();
    printMemory();
    printlnV("STREAM STOPPED");
}

void FirebaseService::startStream()
{
    printlnA("STARTING STREAM");
    printMemory();
    std::string s = "/devices/";
    s.append(_auth->getDeviceId().c_str());
    if (!Firebase.RTDB.beginMultiPathStream(firebaseStreamBdo, s))
    {
        printlnA("------------------------------------");
        printlnA("Can't begin stream connection...");
        printlnA("REASON: " + firebaseStreamBdo->errorReason());
        printlnA("------------------------------------");
        printlnA();
    }
    else
    {
        Firebase.RTDB.setMultiPathStreamCallback(firebaseStreamBdo, streamCallback, streamTimeoutCallback);
        printlnA("STREAM STARTED");
    }

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
            firebaseSemaphore.lockSemaphore("FB onLoop");
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
                _messagingService->getTokens();
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
    _running = r;
}

void FirebaseService::uploadSensorData()
{
    if (_running && Firebase.ready())
    {
        printlnA("Upload sensor data");
        time_t now;
        time(&now);

        char time[40];
        ultoa(now, time, 10);

        String Path = String("/sensorData/") + _auth->getDeviceId() + String("/") + String(time);

        FirebaseJson json;
        bool toUpdate = false;

        for (IFirebaseModule *m : _modules)
        {
            toUpdate = m->updateSensorData(&json) || toUpdate;
        }
        if (!toUpdate)
        {
            printlnA("Nothing to update");
            return;
        }

        String buffer;
        json.toString(buffer, true);
        // printlnD("Uploading sensor data");
        // printlnD(buffer);

        firebaseSemaphore.lockSemaphore("uploadSensorData");
        if (Firebase.RTDB.set(firebaseBdo, Path.c_str(), &json))
        {
            _errorCount = 0;
            printlnD("Sensor data uploaded");
        }
        else
        {
            _errorCount++;
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
        // _lastValidUpdate = millis();
        _errorCount = 0;
    }
    else
    {
        _errorCount++;
        printlnA("FAILED");
        printlnA("REASON: " + firebaseBdo->errorReason());
        printlnA("------------------------------------");
        printlnA();
    }
    firebaseSemaphore.unlockSemaphore();
}

void FirebaseService::factoryReset()
{
    if (_memoryProvider != nullptr)
    {
        printlnA("Factory reset");
        _memoryProvider->factoryReset();
        printlnA("Factory reset done");
        ESP.restart();
    }
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
    printlnD("Firebase valueCallback");
    printlnV("FB Parse value");
    printlnV("Data path = " + data->dataPath);
    printlnV("Data type = " + data->type);

    String dataPath = data->dataPath.substring(1);

    int index = dataPath.indexOf("/");
    int secondIndex = dataPath.indexOf("/", index + 1);

    String settingsKey = dataPath.substring(index + 1, secondIndex);
    printlnV("key = " + settingsKey);
    printV("Modules ");
    printlnV((int)_modules.size());

    for (IFirebaseModule *module : _modules)
    {
        String s = module->getSettingKey();

        printV("Trying module");
        printlnV(s);

        if (s == settingsKey)
        {
            printlnV("Settings key equals");
            module->parseValue(data->dataPath, data->value);
            printMemory();
        }
        else
        {
            printlnV("settings key is not equal");
        }
    }
}

void FirebaseService::uploadState(String key, bool value)
{
    if (_running && Firebase.ready())
    {
        printlnA("FB: Uploading state: " + key);

        String path = String("devices/") + _auth->getDeviceId() + String("/state") + key;
        checkSSLConnected();
        firebaseSemaphore.lockSemaphore("upload state");
        if (Firebase.RTDB.set(firebaseBdo, path.c_str(), value))
        {
            printlnD("State uploaded");
            _errorCount = 0;
        }
        else
        {
            _errorCount++;
            printlnE("State upload FAILED");
            printlnE(firebaseBdo->errorReason());
        }
        firebaseSemaphore.unlockSemaphore();
    }
}

void FirebaseService::uploadState(String key, float value)
{
    if (_running && Firebase.ready())
    {
        printlnA("FB: Uploading state: " + key);
        String path = String("devices/") + _auth->getDeviceId() + String("/state") + key;
        checkSSLConnected();
        firebaseSemaphore.lockSemaphore("uploadState");
        if (Firebase.RTDB.set(firebaseBdo, path.c_str(), value))
        {
            printlnV("State uploaded");
            _errorCount = 0;
        }
        else
        {
            _errorCount++;
            printlnE("State upload FAILED");
            printlnE(firebaseBdo->errorReason());
        }
        firebaseSemaphore.unlockSemaphore();
    }
}
void FirebaseService::uploadState(String key, int value)
{
    if (_running && Firebase.ready())
    {
        printlnA("FB: Uploading state: " + key);
        String path = String("devices/") + _auth->getDeviceId() + String("/state") + key;
        checkSSLConnected();
        firebaseSemaphore.lockSemaphore("uploadState");
        if (Firebase.RTDB.set(firebaseBdo, path.c_str(), value))
        {
            printlnV("State uploaded");
            _errorCount = 0;
        }
        else
        {
            _errorCount++;
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
        printlnA(firebaseBdo->errorReason());
        printlnA("Version not found");
        firebaseSemaphore.unlockSemaphore();
        return "";
    }
}

/**
 * @brief Get download URL for given firmware version
 * 
 * @param version Firmware version
 * @return String Download URL of the new firmware, empty URL if no download link found
 */
String FirebaseService::getFirmwareDownloadUrl(String version)
{

    String fileName = getFirmwareName(version);
    if (fileName == "")
        return "";
    String filePath = "firmware/" + fileName;
    String url;
    firebaseSemaphore.lockSemaphore("getFirmwareDownloadUrl");
    firebaseBdo->setCert(storage_cert);

    if (Firebase.Storage.getMetadata(firebaseBdo, STORAGE_BUCKET_ID, filePath.c_str()))
    {
        url = firebaseBdo->downloadURL();
        printA("GOT URL: ");
        printlnA(url);
    }
    else
    {
        url = "";
        printE("getFirmwareDownloadUrl request failed:");
        printlnE(firebaseBdo->errorReason());
    }
    firebaseBdo->setCert(rtdb_cert);
    firebaseSemaphore.unlockSemaphore();
    return url;
}

void FirebaseService::uploadCustomData(String prefix, String suffix, String data)
{
    if (_running)
    {
        printlnA("uploadCustomData: " + prefix + " - " + suffix);
        String path = prefix + _auth->getDeviceId() + suffix;
        printlnA("Before ssl check");
        checkSSLConnected();
        printlnA("after ssl check");
        firebaseSemaphore.lockSemaphore("uploadCustomData");

        if (Firebase.RTDB.set(firebaseBdo, path.c_str(), data))
        {
            printlnV("custom data uploaded");
            _errorCount = 0;
        }
        else
        {
            _errorCount++;
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
        if (_errorCount > 5)
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
    if (_running && Firebase.ready())
    {
        heap_caps_check_integrity_all(true);
        printlnA("uploadCustomData: " + prefix + " - " + suffix);
        String path = prefix + _auth->getDeviceId() + suffix;

        checkSSLConnected();
        heap_caps_check_integrity_all(true);
        firebaseSemaphore.lockSemaphore("uploadCustomData");
        if (Firebase.RTDB.set(firebaseBdo, path.c_str(), data))
        {
            printlnA("custom data uploaded");
            _errorCount = 0;
        }
        else
        {
            printlnE("custom data upload FAILED");
            _errorCount++;
            printlnE(firebaseBdo->errorReason());
        }
        firebaseSemaphore.unlockSemaphore();
    }
}

void streamCallback(MultiPathStream stream)
{
    printlnA("Stream callback");

    // Check active status
    stream.get(firebaseService->childPaths[ChildPath::ACTIVE_STATUS]);
    if (stream.type == "boolean" && stream.value == "false")
    {
        printlnA("FactoryReset");
        firebaseService->factoryReset();
    }

    //Check firmware
    stream.get(firebaseService->childPaths[ChildPath::FIRMWARE]);

    if (stream.type == "string" && stream.value != "" && otaService != nullptr && otaService->isNewVersion(stream.value))
    {
        String url = firebaseService->getFirmwareDownloadUrl(stream.value);
        if (!url.isEmpty())
        {
            firebaseService->stopFirebase();
            bleController->stop();
            if (!otaService->prepareAndStartUpdate(url, stream.value))
            {
                //If preparation failed, start firebase again
                firebaseService->startFirebase();
            }
        }
    }

    // Check Bluetooth name
    stream.get(firebaseService->childPaths[ChildPath::BLE_NAME]);
    if (stream.type == "string" && (stream.value != ""))
    {
        bleController->setBleName(stream.value);
    }

    for (int i = 0; i < 2; i++)
    {
        stream.get(firebaseService->childPaths[i]);
        if (stream.type == "json")
        {
            FirebaseJson json;
            json.setJsonData(stream.value);
            firebaseService->jsonCallback(&json, stream.dataPath);
        }
        else if (stream.type != "")
        {
            firebaseService->valueCallback(&stream);
        }
    }

    Serial.printf("Received stream payload size: %d (Max. %d)\n\n", stream.payloadLength(), stream.maxPayloadLength());
}

void streamTimeoutCallback(bool timeout)
{
    if (timeout)
    {
        printlnW("FB: WARN - TIMEOUT");
    }
    if (!firebaseStreamBdo->httpConnected())
        Serial.printf("error code: %d, reason: %s\n\n", firebaseStreamBdo->httpCode(), firebaseStreamBdo->errorReason().c_str());
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

void FirebaseService::clearVersion()
{
    String path = String("devices/") + _auth->getDeviceId() + childPaths[ChildPath::FIRMWARE];
    firebaseSemaphore.lockSemaphore("clearVersion");

    if (Firebase.RTDB.setString(firebaseBdo, path.c_str(), "0"))
    {
        printlnV("Version Cleared");
        _errorCount = 0;
    }
    else
    {
        _errorCount++;
        printlnE("Version clear FAILED");
        printlnE(firebaseBdo->errorReason());
    }

    firebaseSemaphore.unlockSemaphore();
}
#include "firebase.h"

#include "cert.h"
#include <HardwareSerial.h>
#include "../auth/auth.h"
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <Firebase_ESP_Client.h>
#include "i_FirebaseModule.h"
#include "../ota/ota.h"
#include "../wifi/wifiProvider.h"
#include "../utils/debug/memory.h"
#include <Esp.h>
#include "../memory/memory_provider.h"
#include "messagingService.h"
#include "../bluetooth/bluetooth.h"
#include "firebaseBdo.h"
#include "semaphore/firebaseSemaphore.h"
#include "sender/sender.h"

#define API_KEY "AIzaSyCrVwfunuauskErYhbScfK4rXh2RTG4v0o"

#define FIREBASE_PROJECT_ID "vivarium-control-unit"
#define FIREBASE_SERVICE_EMAIL "firebase-adminsdk-dg0j0@vivarium-control-unit.iam.gserviceaccount.com"

#define STORAGE_BUCKET_ID "vivarium-control-unit.appspot.com"
#define DATABASE_URL "vivarium-control-unit.firebaseio.com"

#define USER_EMAIL "device@fw.com"
#define USER_PASSWORD "AZE}(vYJxe}Z'3rU"

#define DEFAULT_UPLOAD_DELAY 30000                 //30 second
#define REFRESH_FCM_TOKENS_INTERVAL 60 * 60 * 1000 // every 1 hour refresh FCM tokens
#define FBDO_CLEAR_DELAY 170 * 1000
#define FIREBASE_READY_DELAY 60000
#define FIREBASE_TOKEN_DELAY 10000
#define SERVICE_ACCOUNT_ENABLED false

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

    Serial.println("Firebase begin");
    Firebase.begin(&firebaseConfig, &firebaseAuth);

    Serial.println("Firebase begin done");
    Firebase.reconnectWiFi(true);

    unsigned long start = millis();
    while (!Firebase.ready())
    {
        delay(500);
        Serial.println("Firebase.ready()?");
        if (millis() - start > FIREBASE_TOKEN_DELAY)
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
        _initialized = _initializeFB(SERVICE_ACCOUNT_ENABLED);
    }

    firebaseSender.useLegacyAPI(!SERVICE_ACCOUNT_ENABLED);

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

        bool toUpdate = false;

        for (IFirebaseModule *m : _modules)
        {
            toUpdate = m->updateSensorData(&firebaseSender.json) || toUpdate;
        }
        if (!toUpdate)
        {
            firebaseSender.json.clear();
            printlnA("Nothing to update");
            return;
        }
        else
        {
            firebaseSender.sendJson = true;
        }
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
    printlnA("Log new start");

    time_t now;
    time(&now);
    char timeArray[40];
    ultoa(now, timeArray, 10);

    std::shared_ptr<Data> data = std::make_shared<Data>();
    data->path = "/start/" + std::string(_auth->getDeviceId().c_str()) + "/" + std::string(timeArray);
    data->boolData = true;
    data->type = DataType::BOOL;
    firebaseSender.addData(data);
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
void FirebaseService::valueCallback(String path, String value, String type)
{
    printlnD("Firebase valueCallback");
    printlnV("Data path = " + path);
    printlnV("Data type = " + type);
    printlnV("Data value = " + value);

    String tmpPath = path.substring(1);
    int index = tmpPath.indexOf("/");
    int secondIndex = tmpPath.indexOf("/", index + 1);
    String settingsKey = tmpPath.substring(index + 1, secondIndex);

    printlnV("key = " + settingsKey);
    printV("Modules ");
    printlnV((int)_modules.size());

    for (IFirebaseModule *module : _modules)
    {
        String s = module->getSettingKey();

        printV("Trying module ");
        printlnV(s);

        if (s == settingsKey)
        {
            printlnV("Settings key equals");
            module->parseValue(path, value);
            printMemory();
        }
        else
        {
            printlnV("settings key is not equal");
        }
    }
}

void FirebaseService::uploadState(std::string key, bool value)
{
    if (_running && Firebase.ready())
    {
        printA("FB: Async uplade state: ");
        printlnA(key.c_str());
        checkSSLConnected();
        std::shared_ptr<Data> data = std::make_shared<Data>();
        data->path = "devices/" + std::string(_auth->getDeviceId().c_str()) + "/state" + key;
        data->boolData = value;
        data->type = DataType::BOOL;
        firebaseSender.addData(data);
    }
}

void FirebaseService::uploadState(std::string key, float value)
{
    if (_running && Firebase.ready())
    {
        printA("FB: Async uplade state: ");
        printlnA(key.c_str());
        checkSSLConnected();
        std::shared_ptr<Data> data = std::make_shared<Data>();
        data->path = "devices/" + std::string(_auth->getDeviceId().c_str()) + "/state" + key;
        data->floatData = value;
        data->type = DataType::FLOAT;
        firebaseSender.addData(data);
    }
}

void FirebaseService::uploadState(std::string key, int value)
{
    if (_running && Firebase.ready())
    {
        printA("FB: Async uplade state: ");
        printlnA(key.c_str());
        checkSSLConnected();
        std::shared_ptr<Data> data = std::make_shared<Data>();
        data->path = "devices/" + std::string(_auth->getDeviceId().c_str()) + "/state" + key;
        data->intData = value;
        data->type = DataType::INT;
        firebaseSender.addData(data);
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
    bool retry = true;
    int count = 0;

    while (retry)
    {
        debugA("Try number %d", count);
        if (Firebase.Storage.getMetadata(firebaseBdo, STORAGE_BUCKET_ID, filePath.c_str()))
        {
            url = firebaseBdo->downloadURL();
            printA("GOT URL: ");
            printlnA(url);
            retry = false;
        }
        else
        {
            url = "";
            debugE("getFirmwareDownloadUrl request failed: %s", firebaseBdo->errorReason().c_str());
            count++;
            delay(1500);
        }
        if (count > 6)
            retry = false;
    }
    firebaseSemaphore.unlockSemaphore();
    return url;
}

void FirebaseService::uploadCustomData(std::string prefix, std::string suffix, std::string str)
{
    if (_running && Firebase.ready())
    {
        debugA("Async uploadCustomData P: %s, s: %s, v: %s", prefix.c_str(), suffix.c_str(), str.c_str());
        checkSSLConnected();
        std::shared_ptr<Data> data = std::make_shared<Data>();
        data->path = prefix + std::string(_auth->getDeviceId().c_str()) + suffix;
        data->stringData = str;
        data->type = DataType::STRING;
        firebaseSender.addData(data);
    }
}

void FirebaseService::uploadCustomData(std::string prefix, std::string suffix, float fl)
{
    if (_running && Firebase.ready())
    {
        debugA("Async uploadCustomData P: %s, s: %s, v: %f", prefix.c_str(), suffix.c_str(), fl);
        // printlnA(std::string("Async uploadCustomData: " + prefix + " - " + suffix).c_str());
        checkSSLConnected();
        std::shared_ptr<Data> data = std::make_shared<Data>();
        data->path = prefix + std::string(_auth->getDeviceId().c_str()) + suffix;
        data->floatData = fl;
        data->type = DataType::FLOAT;
        firebaseSender.addData(data);
    }
}

void checkActiveStatus(MultiPathStream stream)
{
    stream.get(firebaseService->childPaths[ChildPath::ACTIVE_STATUS]);
    if (stream.type == "boolean" && stream.value == "false")
    {
        printlnA("FactoryReset");
        firebaseService->factoryReset();
    }
}

void checkFirmwareVersion(MultiPathStream stream)
{
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
}

void checkBluetoothName(MultiPathStream stream)
{
    stream.get(firebaseService->childPaths[ChildPath::BLE_NAME]);
    if (stream.type == "string" && (stream.value != ""))
    {
        bleController->setBleName(stream.value);
    }
}

void checkModules(MultiPathStream stream)
{
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
            firebaseService->valueCallback(stream.dataPath, stream.value, stream.type);
        }
    }
}

void streamCallback(MultiPathStream stream)
{
    printlnA("Stream callback");
    checkActiveStatus(stream);
    checkFirmwareVersion(stream);
    checkBluetoothName(stream);
    checkModules(stream);
    debugA("Received stream payload size: %d (Max. %d)\n\n", stream.payloadLength(), stream.maxPayloadLength());
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
    }
    else
    {
        printlnE("Version clear FAILED");
        printlnE(firebaseBdo->errorReason());
    }

    firebaseSemaphore.unlockSemaphore();
}
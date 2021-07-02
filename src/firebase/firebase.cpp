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

// #define FIREBASE_TOKEN "bd2c2a9af5f31269ce8868e8b0051839"
// #define FIREBASE_SERVICE_ACCOUNT "/service_account_file.json"
// #define FIREBASE_CERT_FILE "/cert.cer"

#define STORAGE_BUCKET_ID "vivarium-control-unit.appspot.com"

#define DATABASE_URL "vivarium-control-unit.firebaseio.com"

#define FIREBASE_PROJECT_ID "vivarium-control-unit"
#define FIREBASE_CLIENT_EMAIL "firebase-adminsdk-dg0j0@vivarium-control-unit.iam.gserviceaccount.com"

#define DEFAULT_UPLOAD_DELAY 60000                 //1 min
#define REFRESH_FCM_TOKENS_INTERVAL 60 * 60 * 1000 // every 1 hour refresh FCM tokens
#define FBDO_CLEAR_DELAY 170 * 1000

FirebaseService *firebaseService;

// void printResult(FirebaseStream &data);
void streamCallback(MultiPathStream data);
void streamTimeoutCallback(bool timeout);

FirebaseAuth auth;
FirebaseConfig config;

//TODO remove later

const char *private_key = "-----BEGIN PRIVATE KEY-----\n"
                          "MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQC+HBEl6IocCh/V\n"
                          "dvWJeDcZ2CSO6CTBRStNlarfh4lMvfHivPKg90mvbVTsq1+ptjAU+PfTcrvMmgDO\n"
                          "MTr7fqDGHL1WrSCvSFkaYzAQuaX6BWBfVHZduCHejrULpDtWjR2jdzI2FY7JX6Nz\n"
                          "cKHJTB4GPbMlmqm7BcMapgtiqPXVr37eZkKErdZhPDc2wI+EWBx59Xh3nfmhy8fD\n"
                          "qZkru+wVaBUqOeSO9AjVmDqz7/BugH24gYrfMyVXdlRuF/v5FEh/UHtvg+07Sbm7\n"
                          "DJTBnOKFL9KGsOnWTeDfJmnjNT7GQ481EY2ncuo+Ot/KXTlIpa6nD4T9ZDBAwEkw\n"
                          "BurgcfEdAgMBAAECggEAL5+1VgNCcdLBdca2rMjeOM2yHtCdwILU3bs0EooQBZcB\n"
                          "vNSrjVJVdapUX3Nw5AFdWyuhXal8zTz5Ha4sgesPWIHDlq6JJQ/hLmCRnmb7Yr4t\n"
                          "DcSJYGHrriaeyPtL2BtCxPvrqqvM2LpqJlWdWeGFFfgn5DAx+8VuQkM9T+pWp0BA\n"
                          "ZmeFUwOxBjy93JfIG7EpdaianOs8fjnUqRtvI4ORKb8VI1cJc8jd9SXjBuQrUmUf\n"
                          "Ei/356jNlpU/rjqcASpAdi9AM3d/qXVrU9sGFaQ3InUPlQfXkERp+779qTIKy79+\n"
                          "XO1pb8PA6+fGx2+s1AOQGy8t22pGNApFUCFcfX0YewKBgQDd1RXXowGY5ZN7Pbrw\n"
                          "Lib2g/1nfrZFHYJMMFy8fIPowgT/Joh7ipE2VAtCpfAj4z3p5V48AWajS41kModt\n"
                          "w36LsW8GxbB5c6E4Bj9GVgleF9LotykymCP0gXxrvRWN9Mk9LruMIx0XsUcly14K\n"
                          "c9ziFAYEU26yyXbaf55DR/IkUwKBgQDbZCUwmAq33+E2A81n9UjK+k+KGWlLCaZh\n"
                          "Zz+KrFqwd4EytvrdZi5bAAthL9tKzOpwGJrJ9NubJzlLHhTUCe+PR/az/82mTWIv\n"
                          "Xszq3Qbu6O9y82FA/57ANiLJSwuNZNwEcULwCsawugVYCWvo3KcduZIw806SXRc/\n"
                          "i3tJXGzmzwKBgBIWrhFHWXn+PmhuQDAVk1fGq4Mk8ffw0A8mYml8PcVdDMtBeR+Z\n"
                          "zP2BHOnyXgKPJR1NdsGt25C1OHJTLHfm2QrLDSKgPCOrKhpHaCF0Io9poekYBmP5\n"
                          "w/TMGjku0fMhYsd6aBClTFoCOqr6SlDP4dMNjvALXZt2khp/DYiu9S/BAoGAJiYv\n"
                          "4VdO5dJkUwQuP5mDYuhL1HO+v3GaIO3XOsHlszHUoYD39m/CN8i2MdwkgclIKt3c\n"
                          "bKnLVhtn9wvwCz7/DScyWvJsTDLAlAQkFeMBRaHzoUV479iDPmErg10tURTsvUkE\n"
                          "nsEA89IlA73/qapU1PJj3WcxjnnphP84HPWZajkCgYAcEsRZqmmzV+D5ZKr9gWlm\n"
                          "HiLzSXMud8RogXQ7TMUstkazdBxXh30opdSSoVRSC/kOnWTwa0TLw18KJ3Q9vkOO\n"
                          "zb2Di2oncdEubuDXxvLYAEZ5NhXWMfUlkPYcw5IDMhwaFynS7gv0wu1/4Y6ClAu7\n"
                          "v4gC8z0U0CM+Pitu5m+8zw==\n"
                          "-----END PRIVATE KEY-----\n";

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

FirebaseData *fbdo;

/* 6. Define the FirebaseAuth data for authentication data */
FirebaseAuth firebaseAuth;

/* 7. Define the FirebaseConfig data for config data */
FirebaseConfig firebaseConfig;

String path = "";

void testFirebase()
{
    // fbdo = new FirebaseData();
    // firebaseBdo = new FirebaseData();

    firebaseConfig.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
    firebaseConfig.service_account.data.project_id = FIREBASE_PROJECT_ID;
    firebaseConfig.service_account.data.private_key = private_key;

    firebaseAuth.token.uid = "";

    firebaseConfig.database_url = DATABASE_URL;

    Firebase.reconnectWiFi(true);
    firebaseBdo->setResponseSize(4096);

    String base_path = "/Test/token";

    firebaseConfig.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    firebaseConfig.max_token_generation_retry = 5;

    Serial.println("Firebase pre - begin");
    Firebase.begin(&firebaseConfig, &firebaseAuth);

    Serial.println("After begin");

    path = base_path + firebaseAuth.token.uid.c_str();
}

//TODO remove above

FirebaseService::FirebaseService(Auth *auth, MemoryProvider *provider, MessagingService *service) : _auth(auth), _memoryProvider(provider), _messagingService(service)
{
    firebaseStreamBdo = new FirebaseData();
    firebaseBdo = new FirebaseData();
    _modules.reserve(8);
    _firebaseMessagingTokens.reserve(5);
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
    if (!_initialized)
    {
        testFirebase();

        // firebaseConfig->cert.file = FIREBASE_CERT_FILE;
        // firebaseConfig->cert.file_storage = mem_storage_type_flash;

        // firebaseConfig.cert.data = cert;
        // firebaseConfig.service_account.data.client_id = "115674499447632950713";
        // config.service_account.data.private_key = private_key;
        // firebaseConfig.service_account.data.private_key_id = "66837c5ae5c45ba17275a615f27382cc1eaac9ef";
        // firebaseAuth.token.uid = ""; //Empty for sign in as admin

        // config.database_url = FIREBASE_RTDB_URL;

        // printMemory();

        // config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
        // config.service_account.data.project_id = FIREBASE_PROJECT_ID;
        // // config.service_account.data.private_key = PRIVATE_KEY;

        // auth.token.uid = "";
        // config.database_url = DATABASE_URL;

        // // config.token_status_callback = tokenStatusCallback;

        // firebaseStreamBdo->setResponseSize(4096);
        // firebaseBdo->setResponseSize(2048);

        // config.max_token_generation_retry = 5;

        // Firebase.reconnectWiFi(true);

        // printlnA("Firebase begin");
        // Firebase.begin(&config, &auth);
        // printlnA("....");

        // Firebase.setFloatDigits(2);
        // Firebase.setDoubleDigits(2);

        // _initialized = true;

        // printMemory();
    }
    unsigned long start = millis();

    while (!Firebase.ready())
    {
        delay(1000);
        printA(".");
        if (millis() - start > 10000)
        {
            printlnE("Token init failed");
            return;
        }
    }
    printlnA("Token init success - Firebase ready");

    checkActiveStatus();
    getFCMSettings();
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
        printlnA("Upload sensor data");
        time_t now;
        time(&now);

        char time[40];
        ultoa(now, time, 10);

        String Path = String("/sensorData/") + _auth->getDeviceId() + String("/") + String(time);

        FirebaseJson json;

        for (IFirebaseModule *m : _modules)
        {
            printA("Updating sensor data->");
            printlnA(m->getSettingKey());
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
    if (_running)
    {

        printlnA("uploading state");

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

        printlnA("uploadCustomData");
        printlnA(prefix);
        printlnA(suffix);
        printlnA("----");
        String path = prefix + _auth->getDeviceId() + suffix;
        printlnA("Before ssl check");
        checkSSLConnected();
        printlnA("after ssl check");
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
        printlnA("uploadCustomData");
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

void streamCallback(MultiPathStream stream)
{
    printlnA("Stream callback");

    // Check active status
    stream.get(firebaseService->childPaths[ChildPath::ACTIVE_STATUS]);
    printlnA("check active status");
    if (stream.type == "boolean" && stream.value == "false")
    {
        printlnA("FactoryReset");
        firebaseService->factoryReset();
    }
    printlnA("active status checked");

    //Check firmware
    stream.get(firebaseService->childPaths[ChildPath::FIRMWARE]);
    printlnA("check firmware");

    if (stream.type == "string" && (stream.value != ""))
    {
        if (otaService != nullptr)
        {
            if (otaService->isNewVersion(stream.value))
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
    }
    printlnA("Firmware checked");

    // Check Bluetooth name
    stream.get(firebaseService->childPaths[ChildPath::BLE_NAME]);
    printlnA("bluetooth name check");
    if (stream.type == "string" && (stream.value != ""))
    {
        bleController->setBleName(stream.value);
    }
    printlnA("ble checked");

    for (int i = 0; i < 2; i++)
    {
        stream.get(firebaseService->childPaths[i]);
        printlnA(stream.dataPath);
        printlnA(stream.type);
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

        if (json.get(data, "/nLimit", false))
        {
            // _notificationCrossLimit = data.boolValue;
            _messagingService->setNotifyOnCrossLimit(data.boolValue);
        }

        if (json.get(data, "/nDelay", false))
        {
            _messagingService->setDelayFCM(data.intValue);
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

        // printA("DELAY = ");
        // printlnA(_delayFCMNotification);
        // printA("limit = ");
        // printlnA(_notificationCrossLimit);
        // printA("connection = ");
        // printlnA(_notificationConnectionOn);
    }
    else
    {

        printlnA("Couldnt receive settings");
    }
    firebaseSemaphore.unlockSemaphore();
}

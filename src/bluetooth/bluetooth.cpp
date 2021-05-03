#include "bluetooth.h"

#include <NimBLEDevice.h>
#include <Esp.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#include "i_bluetooth.h"
#include "esp32-hal-bt.h"

#include "../debug/memory.h"
#include "../wifi/wifiProvider.h"

#define CREDENTIAL_HANDLES 3

BLEController bleController;

//****************

// Service wifi callbacks

//*****************

class BleNameCallbacks : public BLECharacteristicCallbacks
{
private:
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        printI("New ble name: ");
        const char *name = pCharacteristic->getValue().c_str();
        printlnI(name);
        if (strcmp(name, "") == 0)
        {
            bleController.setBleName(name);
        }
        else
        {
            bleController.setBleName(DEFAULT_BLE_NAME);
        }
    }
};

//*****************

// Server callbacks

//*****************

class VivariumServerCallbacks : public NimBLEServerCallbacks
{

    void onConnect(BLEServer *pServer, ble_gap_conn_desc *desc)
    {

        bleController.setConnectionHandle(desc->conn_handle);
        printlnD("OnConnect");
        bleController.setDeviceConnected(true);
    };

    void onDisconnect(BLEServer *pServer)
    {
        printlnD("onDisconnect");
        bleController.setDeviceConnected(false);
    }

    //*****************

    // Security

    //*****************

    bool onConfirmPIN(uint32_t pin)
    {
        printI("onConfirmPIN: ");
        printlnI(pin);
        return false;
    }

    uint32_t onPassKeyRequest()
    {
        printlnI("onPassKeyRequest");

        int passkey = random(100000, 999999);
        printA("passkey = ");
        printlnA(passkey);
        if (bleController.bluetoothPINHandler != nullptr)
        {
            bleController.bluetoothPINHandler->setPINToShow(passkey);
        }
        return passkey;
    }

    void onPassKeyNotify(uint32_t pass_key)
    {
        printI("onPassKeyNotify: ");
        printlnI(pass_key);
        if (bleController.bluetoothPINHandler != nullptr)
        {
            bleController.bluetoothPINHandler->setPINToShow(pass_key);
        }
    }

    bool onSecurityRequest()
    {
        printlnI("onSecurityRequest");
        return true;
    }

    void onAuthenticationComplete(ble_gap_conn_desc *desc)
    {
        printlnI("onAuthenticationComplete");
        if (!desc->sec_state.encrypted)
        {
            NimBLEDevice::getServer()->disconnect(desc->conn_handle);
            printlnW("Encrypt connection failed - disconnecting client");
            return;
        }
        else
        {
            printlnI("Starting BLE work!");
            if (bleController.bluetoothPINHandler != nullptr)
            {
                bleController.bluetoothPINHandler->hidePIN();
            }
        }
    }
};

//****************

// BLE Controller

//****************

BLEController::BLEController()
{
    _modules.reserve(8);
}

void BLEController::init()
{
    // Create the BLE Device
    printlnA("");
    printlnA("Setup Bluetooth device");
    printlnA("");
    printlnA("\tSetting up BLE server");

    _bluetoothName = memoryProvider.loadString(BLE_NAME_KEY, DEFAULT_BLE_NAME);

    NimBLEDevice::init(_bluetoothName.c_str());
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
    NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_SC | BLE_SM_PAIR_AUTHREQ_BOND);

    // Create the BLE Server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new VivariumServerCallbacks());

    // if (secure)
    // {
    // BLESecurity *pSecurity = new NimBLESecurity();
    // pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_ONLY);
    // pSecurity->setCapability(ESP_IO_CAP_OUT);
    // pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
    // }

    setupModuleServices();
    pServer->getAdvertising()->start();
    printlnA("Server started");

    _running = true;
    _initialized = true;
    printMemory();
}

void BLEController::setConnectionHandle(uint16_t h)
{
    conn_handle = h;
}

void BLEController::setupModuleServices()
{
    int settings_handles = 0;
    int state_handles = 0;
    int credential_handles = CREDENTIAL_HANDLES;

    for (IBluetooth *module : _modules)
    {
        int tmp_settings = 0, tmp_credentials = 0, tmp_state = 0;
        module->getHandlesCount(&tmp_settings, &tmp_state, &tmp_credentials);
        credential_handles += tmp_credentials;
        settings_handles += tmp_settings;
        state_handles += tmp_state;
        printlnA("");
        debugA("Settings handles = %i", settings_handles);
        debugA("State handles = %i", state_handles);
        debugA("Credentials handles = %i", credential_handles);
        printlnA("")
    }
    printlnA("Final size");
    debugA("Settings handles = %i", settings_handles);
    debugA("State handles = %i", state_handles);
    debugA("Credentials handles = %i", credential_handles);
    BLEService *credentialsService = pServer->createService(BLEUUID(SERVICE_CREDENTIALS_UUID), credential_handles, CREDENTIALS_ID);
    BLEService *settingsService = pServer->createService(BLEUUID(SERVICE_SETTINGS_UUID), settings_handles, SETTINGS_ID);
    BLEService *stateService = pServer->createService(BLEUUID(SERVICE_STATE_UUID), state_handles, STATE_ID);

    BLECharacteristic *characteristicBleName = credentialsService->createCharacteristic(CHARACTERISTIC_BLE_NAME, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_AUTHEN | NIMBLE_PROPERTY::WRITE_ENC);
    characteristicBleName->setCallbacks(new BleNameCallbacks());

    for (IBluetooth *module : _modules)
    {

        module->setupBLEState(stateService);
        module->setupBLESettings(settingsService);
        module->setupBLECredentials(credentialsService);
    }

    settingsService->start();
    stateService->start();
    credentialsService->start();
}

bool BLEController::isDeviceConnected()
{
    return _deviceConnected;
}

bool BLEController::isDeviceConnecting()
{
    return _deviceConnected && !_oldDeviceConnected;
}

bool BLEController::isDeviceDisconnecting()
{
    return !_deviceConnected && _oldDeviceConnected;
}

void BLEController::onDeviceConnecting()
{
    printlnA("BLE - OnDeviceConnecting");
    for (IBluetooth *module : _modules)
    {
        module->onBLEConnect();
    }

    _oldDeviceConnected = _deviceConnected;
}

void BLEController::onDeviceDisconnecting()
{
    _oldDeviceConnected = _deviceConnected;

    /// Turn off bluetooth is wifi has credentials
    if (wifiProvider.hasCredentials())
    {
        printlnA("Wifi provider has credentials");
        stop();
    }

    /// Turn off bluetooth if name changed - need to initializace bluetooth with new name later
    if (_nameChanged)
    {
        printlnD("Name changed");

        printlnA("Bluetooth disconnect - deinitilazing ESP");
        _nameChanged = false;
        stop();
    }
    printlnA("BLE - OnDeviceDisconnecting");

    /// Let all modules know that blueooth is disconnected - they all can behave differently
    for (IBluetooth *module : _modules)
    {
        module->onBLEDisconnect();
    }

    // Check if wifi is running. If not and bluetooth is turned off, we can turn it on again
    if (!wifiProvider.isConnected() && !_running)
    {
        bleController.init();
    }
}

/**
 * @brief Disconnects from any known client and deinitialize bluetooth (free all memory used by BLE stack)
 * 
 */
void BLEController::stop()
{
    printlnA("Bluetooth Deinitializing");
    if (_running)
    {
        printMemory();
        if (isDeviceConnected())
        {
            // Disconnecting
            pServer->disconnect(conn_handle, 0);
        }
        printlnA("Deinit");
        NimBLEDevice::deinit(true);
        printMemory();
        printlnA("Bluetooth Deinitialized");
        _running = false;
    }
    else
    {
        printlnA("Bluetooth not running - no deinitialization needed");
    }
}

void BLEController::setStartInFuture()
{
    _toStart = true;
}
void BLEController::setStopInFuture()
{
    _toStop = true;
}

void BLEController::checkStop()
{
    if (_toStop)
    {
        stop();
        _toStop = false;
    }
}

void BLEController::restartAdvertising()
{
    pServer->startAdvertising();
}

void BLEController::setDeviceConnected(bool connected)
{
    _deviceConnected = connected;
}

void BLEController::addBluetoothPINHandler(IBluetoothPIN *p)
{
    bluetoothPINHandler = p;
}

void BLEController::checkBluetooth()
{
    if (_running)
    {
        if (isDeviceConnected())
        {
            delay(10); //give bluetooth some time
        }
        if (isDeviceDisconnecting())
        {
            onDeviceDisconnecting();
        }
        if (isDeviceConnecting())
        {
            onDeviceConnecting();
        }
    }
    else
    {
        if (_toStart)
        {
            init();
            _toStart = false;
        }
    }
}

bool BLEController::isRunning()
{
    return _running;
}

void BLEController::addModule(IBluetooth *m)
{
    _modules.push_back(m);
}

void BLEController::setBleName(String s)
{

    if (_bluetoothName != s)
    {
        memoryProvider.saveString(BLE_NAME_KEY, _bluetoothName);
        _nameChanged = true;
        _bluetoothName = s;
    }
}

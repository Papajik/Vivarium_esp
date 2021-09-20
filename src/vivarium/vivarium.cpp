#include "vivarium.h"

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#include <TimeAlarms.h>

#include "../utils/debug/debug.h"
#include "../analog/analog.h"
#include "../memory/memory_provider_internal.h"
#include "../auth/auth.h"
#include "../wifi/wifiProvider.h"
#include "../ota/ota.h"
#include "../bluetooth/bluetooth.h"
#include "../modules/internal/lcd_display/lcd.h"
#include "../modules/internal/lcd_display/textModule.h"
#include "../modules/internal/clock_display/clock_display.h"
#include "../moduleControl/moduleControl.h"
#include "../buttonControl/buttonControl.h"
#include "../led/ledControl.h"
#include "../modules/module.h"
#include "../firebase/firebase.h"
#include "../modules/external/water_temperature/water_temp.h"
#include "../runner/runner.h"

#include "../expander/expander.h"
#include "../modules/internal/outlets/outletController.h"

#include <soc/sens_reg.h>
#include "../utils/debug/memory.h"
#include "../utils/monitor/monitor.h"
#include "../watchdog/watchdog.h"

#include "../firebase/sender/sender.h"

// External modules
#include "../modules/external/ph_probe/ph_probe.h"
#include "../modules/external/water_temperature/water_temp.h"
#include "../modules/external/fan/fan.h"
#include "../modules/external/led/led.h"
#include "../modules/external/feeder/feeder.h"
#include "../modules/external/water_level/water_level.h"
#include "../modules/external/heater/heater.h"
#include "../modules/external/water_pump/water_pump.h"

#include "../modules/external/dht/dht.h"
#include "../modules/external/humidifer/humidifier.h"

#include "esp_log.h"

#define WIFI_RECONNECT_DELAY 60000
#define FIREBASE_RETRY_DELAY 60000

Vivarium::Vivarium() : ClassState("vivarium") {}

void Vivarium::setup(int outletCount, String deviceId)
{

    Serial.begin(115200);
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    printlnA("**** Setup: initializing ...");

    lcdDisplay.begin();
    lcdDisplay.setText({"Smart Vivarium", "Setup"});

    Debugger::setupDebug();
    initAnalog();

    ledControl = new LedControl();

    memoryProvider = new MemoryProviderInternal();
    Debugger::addProvider(memoryProvider);

    wifiProvider = new WiFiProvider(memoryProvider);

    memoryProvider->init();

    auth = new Auth(memoryProvider);
    auth->loadFromNVS();
    auth->setDeviceId(deviceId);

    otaService = new OtaService(memoryProvider, &lcdDisplay);

    bleController = new BLEController(memoryProvider, ledControl, &lcdDisplay);

    messagingService = new MessagingService(auth);

    firebaseService = new FirebaseService(auth, memoryProvider, messagingService);

    moduleControl = new ModuleControl();

    buttonControl = new ButtonControl(firebaseService, moduleControl);

    bleController->addBluetoothPINHandler(&lcdDisplay);
    bleController->addModule(wifiProvider);
    bleController->addModule(auth);
    bleController->addModule(firebaseService);

    outletController = new OutletController(memoryProvider, outletCount);

    firebaseService->addModule(outletController);
    bleController->addModule(outletController);
    wifiProvider->addTextOutput(&lcdDisplay);

    wifiProvider->setupWiFi();

    delay(2000);
    otaService->begin();
    clockDisplay.begin();
    lcdDisplay.setText({"Smart Vivarium", "WiFi set"});
}

void Vivarium::finalize()
{
    lcdDisplay.setText({"Smart Vivarium", "Finalize"});
    if (!otaService->isFirmwareUpdating())
    {
        printlnA("\nFinalize\n");
        // bleController.init();
        printHeapInfo();
        if (wifiProvider->isConnected() && auth->isClaimed())
        {
            lcdDisplay.setText({"Smart Vivarium", "Init FB"});
            printlnA("WiFi connected - initializing firebase");

            firebaseService->setupFirebase();
            firebaseService->logNewStart();
            lcdDisplay.setText({"Smart Vivarium", "FB Ready"});
        }
        else
        {
            lcdDisplay.setText({"Smart Vivarium", "Init BLE"});
            printlnA("Init bluetooth");
            bleController->init();
            lcdDisplay.setText({"Smart Vivarium", "BLE Ready"});
        }

        printHeapInfo();
        runner.addCallback(buttonControl->getCallback());

        expander.begin();
    }
    printMemory();
    monitor.addVivarium(this);
    runner.addCallback(monitor.getCallback());
    watchdog.addVivarium(this);
    runner.addCallback(watchdog.getCallback());
    runner.startRunner();
    firebaseSender.setAuth(auth);
    firebaseSender.start();
    lcdDisplay.setText({"Smart Vivarium", "Ready"});
    delay(2000);
    lcdDisplay.unlockText();
}

void Vivarium::addModule(IModule *m)
{
    moduleControl->addModule(m);
    m->setMemoryProvider(memoryProvider);
    m->setLedControl(ledControl);
}

void Vivarium::addBLEModule(IBluetooth *m)
{
    bleController->addModule(m);
}

void Vivarium::addTextModule(TextModule *m)
{
    m->addTextOutput(&lcdDisplay);
    lcdDisplay.addModule(m);
}

void Vivarium::addFirebaseModule(IFirebaseModule *m)
{
    firebaseService->addModule(m);
    m->addFirebaseService(firebaseService);
    m->addMessagingService(messagingService);
}

void Vivarium::onLoop()
{
    pingAlive();
    setMillis();
    if (!otaService->isFirmwareUpdating())
    {
        mainLoop();
    }
    else
        {
        otaLoop();
        }

    debugHandle();
    }

void Vivarium::restart()
{
    moduleControl->beforeShutdown();
    ESP.restart();
}

void Vivarium::otaLoop()
    {
        otaResponse = otaService->onLoop();
        switch (otaResponse)
        {
        case OTA_COMPLETED:
        restart();
        break;
    case OTA_FAIL:
        if (otaService->failCallback())
        {
            firebaseService->clearVersion();
        }
        printlnA("OTA failed, ESP restart");
        restart();
            break;
        case OTA_CANCEL:
            printlnA("OTA cancelled, finalizing");
            finalize();
            break;
        default:
            break;
        }
    }

void Vivarium::mainLoop()
{
    setStateString("Main loop");

    setStep(1);

    clockDisplay.refreshDisplay();

    setStep(2);

    // Check if Firebase/BLE should stop
    firebaseService->checkStop();
    bleController->checkStop();

    setStep(3);

    // Check wifi and try to restart if necessary
    checkWiFiConenction();
    checkResetFirebase();

    setStep(4);

    outletController->onLoop();
    bleController->checkBluetooth();
    lcdDisplay.onLoop();
    setStep(5);
    pingAlive();
    moduleControl->onLoop();
    pingAlive();
    setStep(6);
    wifiProvider->onLoop();
    firebaseService->onLoop();
    Alarm.delay(0);

    setStep(7);
}

void Vivarium::checkWiFiConenction()
{
    if (!wifiProvider->isConnected() &&
        wifiProvider->hasCredentials() &&
        auth->isClaimed() &&
        !bleController->isRunning() &&
        _lastWifiRetry < WIFI_RECONNECT_DELAY + millis())
    {

        printlnA("Wifi connect retry ");
        _lastWifiRetry = millis();
        if (wifiProvider->connect(3000) == WL_CONNECTED)
        {
            bleController->stop();
            firebaseService->setupFirebase();
        }
    }
}

void Vivarium::checkResetFirebase()
{
    if (wifiProvider->isConnected() &&
        !bleController->isRunning() &&
        !firebaseService->isRunning() &&
        _lastFirebaseRetry < FIREBASE_RETRY_DELAY + millis())
    {
        _lastFirebaseRetry = millis();
        firebaseService->setupFirebase();
    }
}

void Vivarium::createModule(ModuleType type, int position, int outlet)
{

    switch (type)
    {
    case ModuleType::DHT_MODULE:
    {
        DhtModule *dht = new DhtModule(position);
        addModule(dht);
        addFirebaseModule(dht);
        addBLEModule(dht);
        addTextModule(dht);
    }
    break;
    case ModuleType::FAN:
    {
        FanController *fan = new FanController(position);
        addModule(fan);
        addFirebaseModule(fan);
        addBLEModule(fan);
        addTextModule(fan);
    }
    break;
    case ModuleType::FEEDER:
    {
        feederPtr = new Feeder(position);
        addModule(feederPtr);
        addFirebaseModule(feederPtr);
        addBLEModule(feederPtr);
        addTextModule(feederPtr);
    }
    break;
    case ModuleType::HEATER:
    {
        Heater *heater = new Heater(position);
        addModule(heater);
        addFirebaseModule(heater);
        addBLEModule(heater);
        addTextModule(heater);
    }
    break;
    case ModuleType::HUMIDIFIER:
        if (outlet != -1)
        {

            Humidifier *hum = new Humidifier(outlet, position);
            addModule(hum);
            addFirebaseModule(hum);
            addBLEModule(hum);
            addTextModule(hum);
        }

        break;
    case ModuleType::LED:
    {
        ledModulePtr = new LedModule(position);
        addModule(ledModulePtr);
        addFirebaseModule(ledModulePtr);
        addBLEModule(ledModulePtr);
        addTextModule(ledModulePtr);
    }
    break;
    case ModuleType::PH_PROBE:
    {
        PhModule *ph = new PhModule(position);
        addModule(ph);
        addFirebaseModule(ph);
        addBLEModule(ph);
        addTextModule(ph);
    }
    break;
    case ModuleType::WATER_LEVEL:
    {
        WaterLevel *waterLevel = new WaterLevel(position);
        addModule(waterLevel);
        addBLEModule(waterLevel);
        addFirebaseModule(waterLevel);
        addTextModule(waterLevel);
    }
    break;
    case ModuleType::WATER_PUMP:
    {
        WaterPump *waterPump = new WaterPump(position);
        addModule(waterPump);
        addFirebaseModule(waterPump);
        addBLEModule(waterPump);
        addTextModule(waterPump);
    }
    break;
    case ModuleType::WATER_TEMPERATURE:
    {
        WaterTempModule *wt = new WaterTempModule(position);
        addModule(wt);
        addFirebaseModule(wt);
        addBLEModule(wt);
        addTextModule(wt);
    }
    break;

    default:
        break;
    }
}
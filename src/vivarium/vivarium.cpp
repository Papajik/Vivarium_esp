#include "vivarium.h"

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#include <TimeAlarms.h>

#include "../debug/debug.h"
#include "../analog/analog.h"
#include "../memory/memory_provider.h"
#include "../auth/auth.h"
#include "../wifi/wifiProvider.h"
#include "../ota/ota.h"
#include "../bluetooth/bluetooth.h"
#include "../modules/internal/lcd_display/lcd.h"
#include "../modules/internal/lcd_display/aqua_screens.h"
#include "../modules/internal/clock_display/clock_display.h"
#include "../buttonControl/moduleControl/moduleControl.h"
#include "../buttonControl/buttonControl.h"
#include "../led/ledControl.h"
#include "../modules/module.h"
#include "../firebase/firebase.h"
#include "../modules/external/water_temperature/water_temp.h"

#include "../expander/expander.h"
#include "../modules/internal/outlets/outletController.h"

#include <soc/sens_reg.h>
#include "../debug/memory.h"

Vivarium::Vivarium() {}

void Vivarium::setup(String deviceId = DEVICE_ID)
{
    Serial.begin(115200);

    printlnA("**** Setup: initializing ...");
    printlnA("...20...");
    printMemory();
    printHeapInfo();

    setupDebug();
    initAnalog();

    memoryProvider.begin();
    auth.loadFromNVS();
    auth.setDeviceId(deviceId);

    bleController.addBluetoothPINHandler(&lcdDisplay);
    bleController.addModule(&wifiProvider);
    bleController.addModule(&auth);
    bleController.addModule(&firebaseService);

    firebaseService.addModule(&outletController);

    outletController.begin();
    wifiProvider.setupWiFi();
    lcdDisplay.begin();
    setAquariumScreens(&lcdDisplay);
    otaService.begin();
    clockDisplay.begin();
}

void Vivarium::finalize()
{
    if (!otaService.isFirmwareUpdating())
    {
        printlnA("\nFinalize\n");
        // bleController.init();
        printHeapInfo();
        if (wifiProvider.isConnected() && auth.isClaimed())
        {
            printlnA("WiFi connected - initializing firebase");
            firebaseService.setupFirebase();
            firebaseService.logNewStart();
        }
        else
        {
            printlnA("Init bluetooth");
            bleController.init();
        }

        printHeapInfo();
        ledControl.updateLedStatus();
        // moduleControl.start();
        buttonControl.start();

        expander.begin();
    }
    printMemory();
}

LcdDisplay *Vivarium::getDisplay()
{
    return &lcdDisplay;
}

void Vivarium::addModule(IModule *m)
{
    moduleControl.addModule(m);
}

void Vivarium::addBLEModule(IBluetooth *m)
{
    bleController.addModule(m);
}

void Vivarium::addFirebaseModule(IFirebaseModule *m)
{
    firebaseService.addModule(m);
}

void Vivarium::onLoop()
{
    if (!otaService.isFirmwareUpdating())
    {
        if (firebaseService.checkFirebase() != 0)
        {
            moduleControl.beforeShutdown();
            ESP.restart();
        }
        //Check if Firebase should stop
        firebaseService.checkStop();
        outletController.onLoop();

        // CHeck if Bluetooth should stop
        bleController.checkStop();

        bleController.checkBluetooth();
        lcdDisplay.tryToRefreshScreen();
        clockDisplay.refreshDisplay();
        moduleControl.onLoop();
        wifiProvider.onLoop();
        firebaseService.onLoop();
        Alarm.delay(0);
    }
    else
    {
        otaResponse = otaService.onLoop();
        switch (otaResponse)
        {
        case OTA_COMPLETED:
            moduleControl.beforeShutdown();
            ESP.restart();
            break;
        case OTA_CANCEL:
            printlnA("OTA cancelled, finalizing");
            finalize();
            break;
        default:
            break;
        }
    }

    debugHandle();
}

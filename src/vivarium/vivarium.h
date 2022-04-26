/**
* @file vivarium.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief Main class of vivarium.
* @version 1.0
* @date 2021-09-20
* 
* @copyright Copyright (c) 2021
* 
*/

#ifndef _VIVARIUM_H_
#define _VIVARIUM_H_

/*! Libraries*/
#include <Arduino.h>

#include "../state/state_values.h"

#include "../modules/moduleType.h"
#include "../utils/taskHealth/taskHealth.h"
#include "../utils/classState/classState.h"

/*!
* \def DEVICE_ID
* Description
*/
#define DEVICE_ID "esp_test"

class IModule;
class IBluetooth;
class IFirebaseModule;
class MemoryProvider;
class Auth;
class LedControl;
class ModuleControl;
class MessagingService;
class TextModule;

class Vivarium : public TaskHealth, public ClassState
{
public:
    /**
    * @brief Construct a new Vivarium object
    * 
    */
    Vivarium();

    /**
    * @brief Setup new vivarium
    * 
    * @param outletCount Number of 230V outlets
    * @param deviceId Device ID. Used for communication with database.
    */
    void setup(int outletCount, String deviceId);

    /**
    * @brief Finalize setup. Should be called after all modules are set
    * 
    */
    void finalize();

    /**
    * @brief Should be called from loop() function
    * 
    */
    void onLoop();

    /**
    * @brief Creates a Module object of given type.
    * 
    * @param type Type of the Module
    * @param position Position on the Button panel
    * @param outlet Optional - number of outlet used for this module
    */
    void createModule(ModuleType type, int position, int outlet = -1);

    /**
    * @brief Tells modules to do last change and restarts ESP
    * 
    */
    void restart();

    MemoryProvider *memoryProvider;

private:
    void addBLEModule(IBluetooth *m);
    void addFirebaseModule(IFirebaseModule *m);
    void addTextModule(TextModule *m);

    void addModule(IModule *m);
    void mainLoop();
    void otaLoop();

    void checkResetFirebase();
    void checkWiFiConenction();

    MessagingService *messagingService;
    ModuleControl *moduleControl;
    Auth *auth;
    LedControl *ledControl;
    int otaResponse;
    unsigned long _lastWifiRetry = 0;
    unsigned long _lastFirebaseRetry = 0;
};

#endif
/*!
* \file c:\Users\Papi\Documents\Arduino\Vivarium_esp\src\vivarium\vivarium.h
* \author Papaj Michal <papaj.mich@gmail.com>
* \version 0.1
* \date 02/04/2021
* \brief 
* \remarks None
* 
* 
* 
*/

#ifndef _VIVARIUM_H_
#define _VIVARIUM_H_

/*! Importation of librairies*/
#include <Arduino.h>

#include "../state/state_values.h"

#include "../modules/moduleType.h"
#include "../utils/taskHealth/taskHealth.h"

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

class Vivarium : public TaskHealth
{
public:
    Vivarium();
    void setup(int, String);
    void finalize();

    void onLoop();

    void createModule(ModuleType type, int position, int outlet = -1);

    void restart();

    void addBLEModule(IBluetooth *m);
    void addFirebaseModule(IFirebaseModule *m);
    void addTextModule(TextModule *m);

    MemoryProvider *memoryProvider;

private:
    void addModule(IModule *m);
    void mainLoop();
    void otaLoop();

    MessagingService *messagingService;
    ModuleControl *moduleControl;
    Auth *auth;
    LedControl *ledControl;
    int otaResponse;
    unsigned long _lastWifiRetry = 0;
};

#endif
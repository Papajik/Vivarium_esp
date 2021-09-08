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

#include "../utils/taskHealth/taskHealth.h"
/*!
* \def DEVICE_ID
* Description
*/
#define DEVICE_ID "esp_test"

class IModule;
class IBluetooth;
class LcdDisplay;
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
    void setup(String);
    void finalize();
    void addModule(IModule *m);

    void onLoop();

    void addBLEModule(IBluetooth *m);
    void addFirebaseModule(IFirebaseModule *m);
    void addTextModule(TextModule *m);

private:
    void mainLoop();
    void otaLoop();

    MemoryProvider *memoryProvider;
    MessagingService *messagingService;
    ModuleControl *moduleControl;
    Auth *auth;
    LedControl *ledControl;
    int otaResponse;
    unsigned long last = 0;
};

#endif
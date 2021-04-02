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
#include "./vivariumType.h"

/*!
* \def DEVICE_ID
* Description
*/
#define DEVICE_ID "esp_test"

//TODO enable RTTI to use dynamic_cast instead of three duplicit calls of addBleModule, addWifiModule, addModule

class IModule;
class IBluetooth;
class LcdDisplay;
class IFirebaseModule;

class Vivarium
{
public:
    Vivarium();
    void setup(String);
    void finalize();
    void addModule(IModule *m);

    void onLoop();
    LcdDisplay *getDisplay();
    
    void addBLEModule(IBluetooth *m);
    void addFirebaseModule(IFirebaseModule *m);

private:
    int otaResponse;
    unsigned long last = 0;
};

#endif
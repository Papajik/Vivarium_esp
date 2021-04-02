/*!
* \file c:\Users\Papi\Documents\Arduino\Vivarium_esp\vivarium-control-aquarium.ino
* \author Papaj Michal <papaj.mich@gmail.com>
* \version 0.1
* \date 08/01/2021
* \brief 
* \remarks None
*/

// 5.  wifi - jádro zůstává beze změny, mělo by fungovat bez změny pro aqua/terra
// 6. upload data - vlastní h soubor pro obě varianty, v nich jen metoda, která bude vracet json.
// 7. wifiProvider jen zavolá getDataCallback, který v hlavním .ino souboru dostane
// IModule musí implementovat nějakou formu ukládání dat do nvs - nebude se měnit tak často

// TODO - udělat interface pro auth/bluetooth/memory uzel. Udělat volnou vazbu mezi těmito třídami.
// Možná dokonce odstranit auth úplně

//#define DEBUG_DISABLED true
//#define DEBUG_INITIAL_LEVEL DEBUG_LEVEL_VERBOSE
//#define DEBUG_USE_FLASH_F true

#include <Arduino.h>
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#include "src/debug/debug.h"

#include "src/vivarium/vivarium.h"

// External modules
#include "src/modules/external/ph_probe/ph_probe.h"
#include "src/modules/external/water_temperature/water_temp.h"
#include "src/modules/external/fan/fan.h"
#include "src/modules/external/led/led.h"
#include "src/modules/external/feeder/feeder.h"
#include "src/modules/external/water_level/water_level.h"
#include "src/modules/external/heater/heater.h"

#include "src/modules/external/dht/dht.h"
#include "src/modules/external/humidifer/humidifier.h"
// ESP DEBUG
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

Vivarium vivarium;

//Aquarium specific

void setupExternalModules()
{
  printlnA("SETUP EXTERNAL MODULES");

  PhModule *ph = new PhModule();
  WaterTempModule *wt = new WaterTempModule();
  FanController *fan = new FanController();
  LedModule *led = new LedModule();
  Feeder *feeder = new Feeder();
  WaterLevel *waterLevel = new WaterLevel();
  Heater *heater = new Heater();
  vivarium.addModule(ph);
  vivarium.addBLEModule(ph);
  vivarium.addFirebaseModule(ph);

  vivarium.addModule(wt);
  vivarium.addBLEModule(wt);
  vivarium.addFirebaseModule(wt);

  vivarium.addModule(fan);
  vivarium.addBLEModule(fan);
  vivarium.addFirebaseModule(fan);

  vivarium.addModule(led);
  vivarium.addBLEModule(led);
  vivarium.addFirebaseModule(led);

  vivarium.addModule(feeder);
  vivarium.addBLEModule(feeder);
  vivarium.addFirebaseModule(feeder);

  vivarium.addModule(waterLevel);
  vivarium.addBLEModule(waterLevel);
  vivarium.addFirebaseModule(waterLevel);

  vivarium.addModule(heater);
  vivarium.addBLEModule(heater);
  vivarium.addFirebaseModule(heater);

  // DhtModule *dht = new DhtModule(18);
  // dhtDebugPointer = dht;
  // vivarium.addModule(dht);
  // vivarium.addFirebaseModule(dht);
  // vivarium.addBLEModule(dht);

  // Humidifier *hum = new Humidifier(0);
  // vivarium.addModule(hum);
  // vivarium.addFirebaseModule(hum);
  // vivarium.addBLEModule(hum);
}

// *******
// Setup
// ******

void setup()
{
  esp_log_level_set("*", ESP_LOG_VERBOSE);
  vivarium.setup("aquarium_1");
  printlnA("aquarium_1");
  setupExternalModules();
  vivarium.finalize();
}

// ******

// Loop

// ******

void loop()
{
  delay(1000); //TODO remove - for debug only
  vivarium.onLoop();
}

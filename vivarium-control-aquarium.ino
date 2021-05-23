/*!
* \file c:\Users\Papi\Documents\Arduino\Vivarium_esp\vivarium-control-aquarium.ino
* \author Papaj Michal <papaj.mich@gmail.com>
* \version 0.1
* \date 08/01/2021
* \brief 
* \remarks None
*/

// #define DEBUG_DISABLED true

//#define DEBUG_DISABLED true
//#define DEBUG_INITIAL_LEVEL DEBUG_LEVEL_VERBOSE
//#define DEBUG_USE_FLASH_F true

#define UNIT_TEST

#ifdef UNIT_TEST

#include "test/test.h"

#else

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
#include "src/modules/external/water_pump/water_pump.h"

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

  WaterLevel *waterLevel = new WaterLevel(0);
  LedModule *led = new LedModule(1);
  WaterPump *waterPump = new WaterPump(2);
  Feeder *feeder = new Feeder(3);
  FanController *fan = new FanController(4);

  PhModule *ph = new PhModule(5);
  WaterTempModule *wt = new WaterTempModule(6);

  Heater *heater = new Heater(-1);

  // Water temp
  vivarium.addModule(wt);
  vivarium.addBLEModule(wt);
  vivarium.addFirebaseModule(wt);

  // Ph
  vivarium.addModule(ph);
  vivarium.addBLEModule(ph);
  vivarium.addFirebaseModule(ph);

  vivarium.addModule(fan);
  vivarium.addBLEModule(fan);
  vivarium.addFirebaseModule(fan);

  vivarium.addModule(feeder);
  vivarium.addBLEModule(feeder);
  vivarium.addFirebaseModule(feeder);

  vivarium.addModule(waterPump);
  vivarium.addBLEModule(waterPump);
  vivarium.addFirebaseModule(waterPump);

  vivarium.addModule(heater);
  vivarium.addBLEModule(heater);
  vivarium.addFirebaseModule(heater);

  vivarium.addModule(led);
  vivarium.addBLEModule(led);
  vivarium.addFirebaseModule(led);

  vivarium.addModule(waterLevel);
  vivarium.addBLEModule(waterLevel);
  vivarium.addFirebaseModule(waterLevel);

  // DhtModule *dht = new DhtModule(0);
  // vivarium.addModule(dht);
  // vivarium.addFirebaseModule(dht);
  // vivarium.addBLEModule(dht);

  // Humidifier *hum = new Humidifier(0, 2);
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
  vivarium.setup("aquarium_4");
  printlnA("aquarium_4");
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
#endif

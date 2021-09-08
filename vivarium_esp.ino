/*!
* \file c:\Users\Papi\Documents\Arduino\Vivarium_esp\vivarium_esp.ino
* \author Papaj Michal <papaj.mich@gmail.com>
* \version 0.1
* \date 08/01/2021
* \brief 
* \remarks None
*/

// #define UNIT_TEST

#ifdef UNIT_TEST

#include "test/test.h"

#else

#include <Arduino.h>
#include "src/vivarium/vivarium.h"

// ESP DEBUG
// #define LOG_LOCAL_LEVEL A

Vivarium vivarium;

// *******
// Setup
// ******

void setup()
{
  vivarium.setup(2, "aquarium_t");
  vivarium.createModule(ModuleType::WATER_LEVEL, 0);
  vivarium.createModule(ModuleType::LED, 1);
  vivarium.createModule(ModuleType::WATER_PUMP, 2);
  vivarium.createModule(ModuleType::FEEDER, 3);
  vivarium.createModule(ModuleType::FAN, 4);
  vivarium.createModule(ModuleType::PH_PROBE, 5);
  vivarium.createModule(ModuleType::WATER_TEMPERATURE, 6);
  vivarium.createModule(ModuleType::HEATER, -1);
  vivarium.finalize();
}

// ******

// Loop

// ******

void loop()
{
  vivarium.onLoop();
}
#endif

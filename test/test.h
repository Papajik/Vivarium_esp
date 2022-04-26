#line 2 "test.h"

#include <AUnitVerbose.h>

using namespace aunit;

//----------------------------------------------------------------------------
// Module Control
//----------------------------------------------------------------------------

#include "test_moduleControl.h"

//----------------------------------------------------------------------------
// Memory
//----------------------------------------------------------------------------

#include "test_memory.h"

//----------------------------------------------------------------------------
// Module
//----------------------------------------------------------------------------

#include "test_module.h"

//----------------------------------------------------------------------------
// External Modules
//----------------------------------------------------------------------------

#include "modules/test_feeder.h"
#include "modules/test_led.h"
#include "modules/test_water_pump.h"

//----------------------------------------------------------------------------
// Alarm
//----------------------------------------------------------------------------

#include "alarm/base_alarm.h"
#include "alarm/plain_alarm.h"
#include "alarm/payload_alarm.h"

void setup()
{
    delay(1000);          // wait for stability on some boards to prevent garbage Serial
    Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
    Serial.println(F("\n\nTesting of vivarium control unit\n"));
    TestRunner::setTimeout(30);
    // TestRunner::setVerbosity(Verbosity::kAll);
}

void loop()
{
    aunit::TestRunner::run();
}

// #include "../src/modules/external/led/led.h"
// #include "driver/rmt.h"

// test(LED_Default_color)
// {

//     memoryProvider.begin("mock");
//     memoryProvider.factoryReset();

//     /// Check default color

//     LedModule *ledModulePtr = new LedModule();
//     assertEqual(16777215, (int)ledModulePtr->getColor());
//     delete ledModulePtr;
//     memoryProvider.end();
// }

// test(LED_Memory_color)
// {
//     memoryProvider.begin("mock");
//     memoryProvider.factoryReset();
//     memoryProvider.saveInt(SETTINGS_LED_KEY, 123);

//     ledModulePtr = new LedModule();
//     assertEqual(123, (int)ledModulePtr->getColor());
//     delete ledModulePtr;
//     memoryProvider.end();
// }

// test(LED_Handles)
// {
//     memoryProvider.begin("mock");
//     memoryProvider.factoryReset();

// }

// test(LED_Trigger)
// {
// }

// test(LED_Read_characteristic)
// {
// }

// /// 2 triggers from json
// test(LED_Parse_trigger_from_JSON)
// {
// }

// /// Test one existing trigger from json
// test(LED_Parse_Trigger_value)
// {
// }
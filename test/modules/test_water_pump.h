#line 2 "test_water_pump.h"

#include "../../src/modules/external/water_pump/water_pump.h"
#include "../../src/state/state_values.h"
#include "../../src/state/state.h"
#include <Firebase_ESP_Client.h>
#include <ESP32Time.h>
#include <AUnitVerbose.h>

using namespace aunit;

class TestWaterPumpOnce : public TestOnce
{
protected:
    void setup() override
    {
        waterPumpPtr = new WaterPump(1, nullptr);
    }

    void teardown() override
    {
        delete waterPumpPtr;
        stateStorage.clear();
    }

public:
};

testF(TestWaterPumpOnce, water_level_missing)
{
    Serial.println("water_level_missing");
    // delay(3000);
    assertFalse(waterPumpPtr->isConnected());
    waterPumpPtr->setConnected(true, false);
    waterPumpPtr->checkConnectionChange();
    assertTrue(waterPumpPtr->isConnected());
    ((IModule *)waterPumpPtr)->onLoop();
    assertFalse(waterPumpPtr->isConnected());
}

testF(TestWaterPumpOnce, water_level_disconnected)
{
    stateStorage.setValue(STATE_WATER_LEVEL_CONNECTED, false);
    assertFalse(waterPumpPtr->isConnected());
    waterPumpPtr->setConnected(true, false);
    waterPumpPtr->checkConnectionChange();
    assertTrue(waterPumpPtr->isConnected());
    ((IModule *)waterPumpPtr)->onLoop();
    assertFalse(waterPumpPtr->isConnected());
}

testF(TestWaterPumpOnce, water_level_connected)
{
    stateStorage.setValue(STATE_WATER_LEVEL_CONNECTED, true);
    assertFalse(waterPumpPtr->isConnected());
    waterPumpPtr->setConnected(true, false);
    waterPumpPtr->checkConnectionChange();
    assertTrue(waterPumpPtr->isConnected());
    ((IModule *)waterPumpPtr)->onLoop();
    assertTrue(waterPumpPtr->isConnected());
}

testF(TestWaterPumpOnce, water_level_low)
{
    stateStorage.setValue(STATE_WATER_LEVEL_CONNECTED, true);
    stateStorage.setValue(STATE_WATER_LEVEL, (uint32_t)10);
    assertFalse(waterPumpPtr->isConnected());
    waterPumpPtr->setConnected(true, false);
    waterPumpPtr->checkConnectionChange();
    waterPumpPtr->setGoal(12);
    assertTrue(waterPumpPtr->isConnected());
    assertFalse(waterPumpPtr->isRunning());
    ((IModule *)waterPumpPtr)->onLoop();
    assertTrue(waterPumpPtr->isConnected());
    assertTrue(waterPumpPtr->isRunning());
}

testF(TestWaterPumpOnce, water_level_high)
{
    stateStorage.setValue(STATE_WATER_LEVEL_CONNECTED, true);
    stateStorage.setValue(STATE_WATER_LEVEL, (uint32_t)15);
    assertFalse(waterPumpPtr->isConnected());
    waterPumpPtr->setConnected(true, false);
    waterPumpPtr->checkConnectionChange();
    waterPumpPtr->setGoal(12);
    assertTrue(waterPumpPtr->isConnected());
    assertFalse(waterPumpPtr->isRunning());
    ((IModule *)waterPumpPtr)->onLoop();
    assertTrue(waterPumpPtr->isConnected());
    assertFalse(waterPumpPtr->isRunning());
}

testF(TestWaterPumpOnce, water_level_invalid)
{
    stateStorage.setValue(STATE_WATER_LEVEL_CONNECTED, true);
    stateStorage.setValue(STATE_WATER_LEVEL, (uint32_t)WATER_LEVEL_INVALID_VALUE);
    assertFalse(waterPumpPtr->isConnected());
    waterPumpPtr->setConnected(true, false);
    waterPumpPtr->checkConnectionChange();
    waterPumpPtr->setGoal(12);
    assertTrue(waterPumpPtr->isConnected());
    assertFalse(waterPumpPtr->isRunning());
    ((IModule *)waterPumpPtr)->onLoop();
    assertTrue(waterPumpPtr->isConnected());
    assertFalse(waterPumpPtr->isRunning());
}

testF(TestWaterPumpOnce, water_level_change)
{
    stateStorage.setValue(STATE_WATER_LEVEL_CONNECTED, true);
    stateStorage.setValue(STATE_WATER_LEVEL, (uint32_t)10);
    assertFalse(waterPumpPtr->isConnected());

    waterPumpPtr->setConnected(true, false);
    waterPumpPtr->checkConnectionChange();
    waterPumpPtr->setGoal(12);
    assertTrue(waterPumpPtr->isConnected());
    assertFalse(waterPumpPtr->isRunning());

    ((IModule *)waterPumpPtr)->onLoop();
    assertTrue(waterPumpPtr->isConnected());
    assertTrue(waterPumpPtr->isRunning());

    stateStorage.setValue(STATE_WATER_LEVEL, (uint32_t)15);
    ((IModule *)waterPumpPtr)->onLoop();
    assertTrue(waterPumpPtr->isConnected());
    assertFalse(waterPumpPtr->isRunning());
}

testF(TestWaterPumpOnce, main_task_freeze)
{
    Serial.println("\n\nmain_task_freeze");
    assertFalse(waterPumpPtr->isFailSafeTriggered());
    stateStorage.setValue(STATE_WATER_LEVEL_CONNECTED, true);
    stateStorage.setValue(STATE_WATER_LEVEL, (uint32_t)10);
    waterPumpPtr->setGoal(12);

    assertFalse(waterPumpPtr->isConnected());
    waterPumpPtr->setConnected(true, false);
    waterPumpPtr->checkConnectionChange();
    assertTrue(waterPumpPtr->isConnected());
    ((IModule *)waterPumpPtr)->onLoop();
    assertTrue(waterPumpPtr->isConnected());
    assertTrue(waterPumpPtr->isRunning());
    stateStorage.removeItem(STATE_WATER_LEVEL, item_type::type_uint32);
    delay(1500);
    assertFalse(waterPumpPtr->isFailSafeTriggered());
    delay(1000);
    assertTrue(waterPumpPtr->isFailSafeTriggered());
    ((IModule *)waterPumpPtr)->onLoop();
    assertFalse(waterPumpPtr->isRunning());
}

testF(TestWaterPumpOnce, main_task_delay)
{
    Serial.println("\n\nmain_task_delay");
    assertFalse(waterPumpPtr->isFailSafeTriggered());
    stateStorage.setValue(STATE_WATER_LEVEL_CONNECTED, true);
    stateStorage.setValue(STATE_WATER_LEVEL, (uint32_t)10);
    waterPumpPtr->setGoal(12);

    assertFalse(waterPumpPtr->isConnected());
    waterPumpPtr->setConnected(true, false);
    waterPumpPtr->checkConnectionChange();
    assertTrue(waterPumpPtr->isConnected());
    ((IModule *)waterPumpPtr)->onLoop();
    assertTrue(waterPumpPtr->isConnected());
    assertTrue(waterPumpPtr->isRunning());

    delay(1500);
    assertFalse(waterPumpPtr->isFailSafeTriggered());
    ((IModule *)waterPumpPtr)->onLoop();
    assertTrue(waterPumpPtr->isRunning());
}

testF(TestWaterPumpOnce, main_task_continue)
{
    Serial.println("\n\nmain_task_delay");
    assertFalse(waterPumpPtr->isFailSafeTriggered());
    stateStorage.setValue(STATE_WATER_LEVEL_CONNECTED, true);
    stateStorage.setValue(STATE_WATER_LEVEL, (uint32_t)10);
    waterPumpPtr->setGoal(12);

    assertFalse(waterPumpPtr->isConnected());
    waterPumpPtr->setConnected(true, false);
    waterPumpPtr->checkConnectionChange();
    assertTrue(waterPumpPtr->isConnected());
    ((IModule *)waterPumpPtr)->onLoop();
    assertTrue(waterPumpPtr->isConnected());
    assertTrue(waterPumpPtr->isRunning());

    delay(1500);
    ((IModule *)waterPumpPtr)->onLoop();
    delay(1500);
    ((IModule *)waterPumpPtr)->onLoop();
    delay(1500);

    assertFalse(waterPumpPtr->isFailSafeTriggered());
    ((IModule *)waterPumpPtr)->onLoop();
    assertTrue(waterPumpPtr->isRunning());
}

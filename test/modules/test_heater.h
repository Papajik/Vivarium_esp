#line 2 "test_heater.h"

#include "../../src/modules/external/heater/heater.h"

#include <Firebase_ESP_Client.h>
#include <ESP32Time.h>
#include <AUnitVerbose.h>

using namespace aunit;

class TestHeaterOnce : public TestOnce
{
protected:
    void setup() override
    {
        heaterPtr = new Heater(1, nullptr);
    }

    void teardown() override
    {
        delete heaterPtr;
    }

public:
};

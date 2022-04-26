#include "outletController.h"
#include "../../../expander/expander.h"
#include "../../../memory/memory_provider.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

OutletController *outletController;

#define OUTLET_MAX 4

#define OUTLET_KEY "o"

OutletController::OutletController(MemoryProvider *provider, int outletCount) : _memoryProvider(provider)
{

    _outletCount = outletCount > OUTLET_MAX ? OUTLET_MAX : outletCount;

    _reserved.reserve(_outletCount);
    _outletChanged.reserve(_outletCount);
    _outlets.reserve(_outletCount);

    for (int i = 0; i < _outletCount; i++)
    {
        expander.pinMode(i, OUTPUT, LOW);
        expander.pinMode(i + _outletCount, OUTPUT, HIGH);

        _reserved.push_back(false);
        _outletChanged.push_back(false);
        _outlets.push_back(false);
    }

    if (expander.begin())
    {
        printlnA("Expander OK");
    }
    else
    {
        printlnA("Expander not OK");
    }
}

void OutletController::setOutlet(int outlet, bool on)
{
    if (outlet < 0 || outlet >= _outletCount || _reserved[outlet])
    {
        printlnA("Outlet is reserved or out of bounds");
        return;
    }

    printlnA("Outlet control ");
    printA("Socket ");
    printA(outlet);
    printA(" is ");
    printlnA(on ? "ON" : "OFF");
    printA("Outlet pin: ");
    printlnA(_outletCount - outlet - 1);
    printA("LED pin: ");
    printlnA(_outletCount * 2 - outlet - 1);

    _outlets[outlet] = on;
    _outletChanged[outlet] = true;
    _memoryProvider->saveBool(OUTLET_KEY + String(outlet), on);
}

bool OutletController::isOutletOn(int outlet)
{
    return _outlets[outlet];
}

void OutletController::onLoop()
{
    updateOutletState();
}

void OutletController::updateOutletState()
{
    for (int i = 0; i < _outletCount; i++)
    {
        if (_outletChanged[i])
        {
            Serial.print("outlet changed = ");
            Serial.println(_outletChanged[i]);
            _outletChanged[i] = false;

            // Outlet
            Serial.println("Outlet " + String(i) + " ( " + String(_outletCount - i - 1) + ") is set to " + String(_outlets[i] ? HIGH : LOW));
            expander.digitalWrite(_outletCount - i - 1, _outlets[i] ? HIGH : LOW);

            // LED
            Serial.println("LED " + String(i) + " ( " + String(_outletCount * 2 - i - 1) + ") is set to " + String(_outlets[i] ? LOW : HIGH));
            expander.digitalWrite(_outletCount * 2 - i - 1, _outlets[i] ? LOW : HIGH);

            Serial.println("writing done");
        }
    }
}

bool OutletController::reserveOutlet(int outlet)
{
    if (outlet >= 0 && outlet < _outletCount)
    {
        if (_reserved[outlet])
        {
            return false;
        }
        else
        {
            _reserved[outlet] = true;
            return true;
        }
    }
    return false;
}
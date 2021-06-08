#include "outletController.h"
#include "../../../expander/expander.h"
#include "../../../memory/memory_provider.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

OutletController *outletController;

#define OUTLET_KEY "o"

OutletController::OutletController(MemoryProvider *provider) : _memoryProvider(provider)
{
    expander.pinMode(P0, OUTPUT, LOW);
    expander.pinMode(P1, OUTPUT, LOW);
    expander.pinMode(P2, OUTPUT, HIGH);
    expander.pinMode(P3, OUTPUT, HIGH);

    if (expander.begin())
    {
        printlnA("Expander OK");
    }
    else
    {
        printlnA("Expander not OK");
    }

    for (int i = 0; i < OUTLET_COUNT; i++)
    {
        setOutlet(i, _memoryProvider->loadBool(OUTLET_KEY + String(i), false));
    }
    onLoop();
}

void OutletController::setOutlet(int socket, bool on)
{
    printlnA("Outlet controll");
    printA("Socket ");
    printA(socket);
    printA(" is ");
    printlnA(on ? "ON" : "OFF");
    printA("Outlet pin: ");
    printlnA(OUTLET_COUNT - socket - 1);
    printA("LED pin: ");
    printlnA(OUTLET_COUNT * 2 - socket - 1);

    _outlets[socket] = on;
    _outletChanged[socket] = true;
    _memoryProvider->saveBool(OUTLET_KEY + String(socket), on);
}

bool OutletController::isOutletOn(int socket)
{
    return _outlets[socket];
}

void OutletController::onLoop()
{
    updateOutletState();
}

void OutletController::updateOutletState()
{
    for (int i = 0; i < OUTLET_COUNT; i++)
    {
        if (_outletChanged[i])
        {
            _outletChanged[i] = false;
            // Outlet
            expander.digitalWrite(OUTLET_COUNT - i - 1, _outlets[i] ? HIGH : LOW);

            // LED
            expander.digitalWrite(OUTLET_COUNT * 2 - i - 1, _outlets[i] ? LOW : HIGH);

        }
    }
}

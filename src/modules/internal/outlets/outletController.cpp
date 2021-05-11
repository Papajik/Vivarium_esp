#include "outletController.h"
#include "../../../expander/expander.h"
#include "../../../memory/memory_provider.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

OutletController *outletController;

#define OUTLET_KEY "o"

OutletController::OutletController(MemoryProvider *provider)
{
    _memoryProvider = provider;
    expander.pinMode(P7, OUTPUT);
    expander.pinMode(P6, OUTPUT);
    expander.pinMode(P5, OUTPUT);
    expander.pinMode(P4, OUTPUT);

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
    printlnI("Outlet controll");
    printI("Socket ");
    printI(socket);
    printI(" is ");
    printlnI(on ? "ON" : "OFF");
    printI("Outlet pin: ");
    printlnI(5 - socket);
    printI("LED pin: ");
    printlnI(7 - socket);

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
    for (int i = 0; i < OUTLET_COUNT; i++)
    {
        if (_outletChanged[i])
        {
            _outletChanged[i] = false;
            // Outlet
            expander.digitalWrite(5 - i, _outlets[i] ? 1 : 0);

            // LED
            expander.digitalWrite(7 - i, _outlets[i] ? 0 : 1);
        }
    }
}

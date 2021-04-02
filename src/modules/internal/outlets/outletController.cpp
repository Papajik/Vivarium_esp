#include "outletController.h"
#include "../../../expander/expander.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

OutletController outletController;

OutletController::OutletController()
{
}

void OutletController::begin()
{
    expander.pinMode(P0, OUTPUT);
    expander.pinMode(P1, OUTPUT);

    if (expander.begin())
    {
        printlnA("Expander OK");
    }
    else
    {
        printlnA("Expander not OK");
    }
}

void OutletController::setOutlet(int socket, bool on)
{
    printlnV("Outlet controll");
    printV("Socket ");
    printV(socket);
    printV(" is ");
    printlnV(on ? "ON" : "OFF");

    _sockets[socket] = on;
    expander.digitalWrite(socket, on ? 1 : 0);
}

bool OutletController::isSocketOn(int socket)
{
    return _sockets[socket];
}
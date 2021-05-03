#include "module.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <HardwareSerial.h>
#include "../buttonControl/moduleControl/moduleControl.h"
#include "../led/ledControl.h"

IModule::IModule(String connectionKey)
{
    _connectionKey = connectionKey;
    loadConnectionState();
}

IModule::~IModule()
{
}

bool IModule::isConnected()

{
    return _connected;
}

void IModule::setConnected(bool connected, bool fromButton)
{
    printA("Module " + _connectionKey);
    printA(" - set connected: ");
    printlnA(connected ? "true" : "false");
    
    
    _connected = connected;
    _sourceIsButton = fromButton;
    memoryProvider.saveBool(_connectionKey, connected);
}

void IModule::checkConnectionChange()
{
    if (_connected != _lastConnected)
    {

        _lastConnected = _connected;
        ledControl.updateLedStatus();
        onConnectionChange();
    }
}

void IModule::loadConnectionState()
{

    _connected = memoryProvider.loadBool(_connectionKey, false);
    if (_connected)
    {
        printlnV(_connectionKey + " connected");
    }
    else
    {
        printlnV(_connectionKey + " disconnected");
    }
}

void IModule::beforeShutdown() {}
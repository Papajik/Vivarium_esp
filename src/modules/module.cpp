#include "module.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <HardwareSerial.h>

#include "../led/ledControl.h"

IModule::IModule(String connectionKey, int position)
{
    _connectionKey = connectionKey;
    _position = position;
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
    _memoryProvider->saveBool(_connectionKey, connected);
    _ledControl->updateLedStatus(_position, connected);
}

void IModule::checkConnectionChange()
{
    if (_connected != _lastConnected)
    {

        _lastConnected = _connected;
        _ledControl->updateLedStatus(_position, _connected);
        onConnectionChange();
    }
}

void IModule::loadConnectionState()
{

    _connected = _memoryProvider->loadBool(_connectionKey, false);
    if (_connected)
    {
        printlnV(_connectionKey + " connected");
    }
    else
    {
        printlnV(_connectionKey + " disconnected");
    }
}

void IModule::setMemoryProvider(MemoryProvider *provider)
{
    _memoryProvider = provider;
    loadConnectionState();
    loadSettings();
    
}

void IModule::setLedControl(LedControl *ledControl)
{
    _ledControl = ledControl;
}

void IModule::beforeShutdown() {}
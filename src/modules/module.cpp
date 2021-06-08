#include "module.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <HardwareSerial.h>

#include "../led/ledControl.h"

IModule::IModule(String connectionKey, int position)
    : _position(position),
      _connectionKey(connectionKey)
{
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
    printD("Module " + _connectionKey);
    printD(" - set connected: ");
    printlnD(connected ? "true" : "false");

    _connected = connected;
    _sourceIsButton = fromButton;
    if (_memoryProvider != nullptr)
        _memoryProvider->saveBool(_connectionKey, connected);

    if (_ledControl != nullptr)
    {
        connected ? _ledControl->setLedOn(_position) : _ledControl->setLedOff(_position);
    }
    // _ledControl->updateLedStatus(_position, connected);
}

void IModule::checkConnectionChange()
{
    if (_connected != _lastConnected)
    {
        _lastConnected = _connected;
        if (_ledControl != nullptr)
        {
            _connected ? _ledControl->setLedOn(_position) : _ledControl->setLedOff(_position);
        }
        onConnectionChange();
    }
}

void IModule::loadConnectionState()
{
    if (_memoryProvider != nullptr)
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

uint8_t IModule::getPosition()
{
    return _position;
}
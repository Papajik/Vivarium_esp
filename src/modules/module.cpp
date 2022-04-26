/**
* @file module.cpp
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/
#include "module.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <HardwareSerial.h>

#include "../led/ledControl.h"

IModule::IModule(String connectionKey, int position, MemoryProvider *provider)
    : ClassState(connectionKey),
      _position(position),
      _connectionKey(connectionKey),
      _memoryProvider(provider)
{
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
    debugD("Module %s - set connected %s", _connectionKey.c_str(), connected ? "true" : "false");
    _connected = connected;
    _sourceIsButton = fromButton;
}

void IModule::checkConnectionChange()
{
    if (_connected != _lastConnected)
    {
        _lastConnected = _connected;
        if (_memoryProvider != nullptr)
            _memoryProvider->saveBool(_connectionKey, _connected);
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


void IModule::setLedControl(LedControl *ledControl)
{
    _ledControl = ledControl;
}

void IModule::beforeShutdown() {}

uint8_t IModule::getPosition()
{
    return _position;
}
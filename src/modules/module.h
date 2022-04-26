/**
* @file module.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef _MODULE_H_
#define _MODULE_H_

#include "../state/state.h"
#include "../memory/memory_provider.h"

class FirebaseService;
class LedControl;

class IModule
{
public:
    IModule(String, int, MemoryProvider *);
    virtual ~IModule();

    bool isConnected();
    void setConnected(bool, bool);

    virtual void beforeShutdown();

    void checkConnectionChange();

    virtual void onConnectionChange() = 0;

    virtual void onLoop() = 0;

    virtual void saveSettings() = 0;
    virtual bool loadSettings() = 0;


    void setLedControl(LedControl *ledControl);

    uint8_t getPosition();

protected:
    uint8_t _position;
    String _connectionKey;

    LedControl *_ledControl = nullptr;
    MemoryProvider *_memoryProvider = nullptr;
    bool _connected = false;
    bool _lastConnected = false;
    bool _sourceIsButton = false;

private:
    void loadConnectionState();
};

#endif
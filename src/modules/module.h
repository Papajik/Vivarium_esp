#ifndef _MODULE_H_
#define _MODULE_H_

#include "../state/state.h"
#include "../memory/memory_provider.h"

class FirebaseService;
class LedControl;

class IModule
{
public:
    IModule(String, int);
    virtual ~IModule();

    bool isConnected();
    void setConnected(bool, bool);

    virtual void beforeShutdown();

    void checkConnectionChange();

    virtual void onConnectionChange() = 0;

    virtual void onLoop() = 0;

    virtual void saveSettings() = 0;
    virtual bool loadSettings() = 0;

    void setMemoryProvider(MemoryProvider *provider);

    void setLedControl(LedControl *ledControl);

    uint8_t getPosition();

protected:
    uint8_t _position;

    LedControl *_ledControl = nullptr;
    MemoryProvider *_memoryProvider = nullptr;
    bool _connected = false;
    bool _lastConnected = false;
    bool _sourceIsButton = false;
    String _connectionKey;

private:
    void loadConnectionState();
};

#endif
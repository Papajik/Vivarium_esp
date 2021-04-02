#ifndef _MODULE_H_
#define _MODULE_H_

#include "../state/state.h"
#include "../memory/memory_provider.h"
class IModule
{
public:
    IModule(String);
    ~IModule();

    bool isConnected();
    void setConnected(bool, bool);

    virtual void beforeShutdown();

    void checkConnectionChange();

    virtual void onConnectionChange() = 0;

    virtual void onLoop() = 0;

    virtual void saveSettings() = 0;
    virtual bool loadSettings() = 0;

    virtual bool isFModule() { return false; }
    virtual bool isBModule() { return false; }

protected:
    bool _connected = false;
    bool _lastConnected = false;
    bool _sourceIsButton = false;
    String _connectionKey;

private:
    void loadConnectionState();
};

#endif
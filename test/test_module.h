#include <AUnitVerbose.h>

#include "../src/modules/module.h"
#include "../src/memory/memory_provider.h"
#include "../src/led/ledControl.h"

using namespace aunit;

class MockModule : public IModule
{
public:
    MockModule(String connectionKey, int position) : IModule(connectionKey, position)
    {
    }

    ~MockModule(){

    }

    virtual void onConnectionChange()
    {
        onConnectionChangeCalled = true;
    }

    virtual void onLoop() {}

    virtual void saveSettings() {}
    virtual bool loadSettings() { return true; }

    bool onConnectionChangeCalled = false;

private:
};

class MockMemory : public MemoryProvider
{
public:
    MockMemory() : MemoryProvider(){};
    virtual ~MockMemory(){};
    virtual void begin(String name = "vivarium"){};
    virtual void end(){};

    // Extern modules settings
    virtual void saveStruct(String key, const void *, size_t){};
    virtual bool loadStruct(String key, void *, size_t) { return true; };

    //Writes
    virtual void saveString(String key, String value){};
    virtual String loadString(String key, String defaultValue) { return defaultValue; };

    virtual void saveBool(String key, bool value)
    {
        savedValue = value;
        usedKeyToSave = key;
    };
    virtual bool loadBool(String key, bool defaultValue)
    {
        if (key.equals(wantedKey))
            return connected;
        return defaultValue;
    };

    virtual void saveInt(String key, uint32_t value){};
    virtual uint32_t loadInt(String key, uint32_t defaultValue) { return defaultValue; }

    virtual void saveFloat(String key, float value){};
    virtual float loadFloat(String key, float defaultValue) { return defaultValue; }

    virtual void saveDouble(String key, double value){};
    virtual float loadDouble(String key, double defaultValue) { return defaultValue; };

    virtual void removeKey(String key){};
    virtual void factoryReset(){};

    int getWriteCount() { return _writeCount; }
    int getBytesWritten() { return _bytesWritten; }

    bool connected = false;
    String wantedKey = "";

    String usedKeyToSave = "";
    bool savedValue = false;

private:
    void _incrementWrites() { _writeCount++; };
    void _incrementBytes(int bytes) { _bytesWritten += bytes; };
};

class LedMock : public LedControl
{
private:
    void updateLedStatus(uint8_t pin, uint8_t status) override
    {
        usedPin = pin;
        savedStatus = status;
    }

public:
    LedMock() : LedControl() {}
    ~LedMock() {}

    uint8_t getLedStatus(uint8_t) override
    {
        return savedStatus;
    }

    void setLedOff(uint8_t position) override
    {
        updateLedStatus(position, 1);
    }

    void setLedOn(uint8_t position) override
    {
        updateLedStatus(position, 0);
    }

    uint8_t usedPin = -1;
    uint8_t savedStatus = -1;
};

test(moduleInit)
{
    IModule *module = new MockModule("", 0);
    assertFalse(module->isConnected());
    delete module;
}

// Default connection is false
test(moduleConnectionDefaultConnectionFromMemory)
{
    IModule *module = new MockModule("key", 0);
    MockMemory *provider = new MockMemory();
    provider->connected = true;
    provider->wantedKey = "differentKey";
    module->setMemoryProvider(provider);
    assertFalse(module->isConnected());
    delete module;
    delete provider;
}

test(moduleConnectionTrueConnectionFromMemory)
{
    IModule *module = new MockModule("key", 0);
    MockMemory *provider = new MockMemory();
    provider->connected = true;
    provider->wantedKey = "key";
    module->setMemoryProvider(provider);
    assertTrue(module->isConnected());
    delete module;
    delete provider;
}

test(moduleCheckConnectionChange)
{
    MockModule *module = new MockModule("key", 0);
    IModule *m = module;
    m->setConnected(true, true);
    assertFalse(module->onConnectionChangeCalled);
    m->checkConnectionChange();
    assertTrue(module->onConnectionChangeCalled);
}

test(moduleConnectionChangeWithMemory)
{
    IModule *module = new MockModule("memoryKey", 0);
    MockMemory *provider = new MockMemory();
    provider->savedValue = false;
    module->setMemoryProvider(provider);
    module->setConnected(true, false);
    assertEqual("memoryKey", provider->usedKeyToSave);
    assertEqual(true, provider->savedValue);
    module->setConnected(false, false);
    assertEqual(false, provider->savedValue);
    delete module;
    delete provider;
}

test(moduleConnectionWithLed)
{
    IModule *module = new MockModule("memoryKey", 3);
    LedMock *led = new LedMock();

    module->setLedControl(led);
    module->setConnected(true, false);

    assertEqual(led->usedPin, 3);
    assertEqual(led->savedStatus, 0);

    module->setConnected(false, false);
    assertEqual(led->savedStatus, 1);

    delete module;
    delete led;
}

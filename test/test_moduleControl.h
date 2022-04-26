#include "../src/moduleControl/moduleControl.h"
#include "../src/modules/module.h"
#include <AUnitVerbose.h>

using namespace aunit;

class ModuleControlOnce : public TestOnce
{
protected:
    void setup() override
    {
    }

    void teardown() override
    {
    }

public:
    ModuleControl moduleControl;
};

class MockModuleMC : public IModule
{
public:
    MockModuleMC(String connectionKey, int position) : IModule(connectionKey, position, nullptr)
    {
    }

    virtual void onConnectionChange() {}

    virtual void onLoop() {}

    virtual void beforeShutdown()
    {
        beforeShutdownCalled = true;
    }

    virtual void saveSettings() {}
    virtual bool loadSettings() { return true; }

    bool beforeShutdownCalled = false;

private:
};

testF(ModuleControlOnce, initialTest)
{
    assertEqual(0, moduleControl.moduleCount());
    assertFalse(moduleControl.isModuleConnected(0));
}

testF(ModuleControlOnce, addModuleTest)
{
    assertEqual(0, moduleControl.moduleCount());
    IModule *module = new MockModuleMC("", 2);
    int size = moduleControl.addModule(module);
    assertEqual(1, size);
    assertEqual(1, moduleControl.moduleCount());
    module->setConnected(true, false);
    assertTrue(moduleControl.isModuleConnected(2));
    module->setConnected(false, false);
    assertFalse(moduleControl.isModuleConnected(2));
    delete module;
}

testF(ModuleControlOnce, isConnectedTest)
{
    IModule *module = new MockModuleMC("", 1);
    moduleControl.addModule(module);
    module->setConnected(true, false);
    assertTrue(moduleControl.isModuleConnected(1));
    module->setConnected(false, false);
    assertFalse(moduleControl.isModuleConnected(1));
    delete module;
}

testF(ModuleControlOnce, isConnectedWrongModuleTest)
{
    IModule *module = new MockModuleMC("", 3);
    IModule *module2 = new MockModuleMC("", 6);
    moduleControl.addModule(module);
    moduleControl.addModule(module2);
    module->setConnected(true, false);
    assertFalse(moduleControl.isModuleConnected(6));
    module->setConnected(false, false);
    assertFalse(moduleControl.isModuleConnected(6));
    delete module;
}

testF(ModuleControlOnce, buttonPressed)
{
    IModule *module = new MockModuleMC("", 0);
    moduleControl.addModule(module);
    module->setConnected(true, false);
    assertTrue(moduleControl.isModuleConnected(0));
    moduleControl.buttonPressed(1);
    assertTrue(moduleControl.isModuleConnected(0));
    moduleControl.buttonPressed(0);
    assertFalse(moduleControl.isModuleConnected(0));
    delete module;
}

testF(ModuleControlOnce, beforeS)
{
    MockModuleMC *module = new MockModuleMC("", 0);
    moduleControl.addModule(module);
    moduleControl.beforeShutdown();
    assertTrue(module->beforeShutdownCalled);
    delete module;
}

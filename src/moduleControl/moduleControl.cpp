#include "moduleControl.h"

#include "../modules/module.h"
#include "../analog/analog.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug



ModuleControl::ModuleControl()
{

    _modules.reserve(8);
}

int ModuleControl::addModule(IModule *module)
{
    _modules.push_back(module);
    return _modules.size();
}

void ModuleControl::buttonPressed(int pressed)
{

    if (pressed >= 0 && pressed < _modules.size())
    {
        IModule *m = _modules[pressed];
        if (m != nullptr)
        {
            m->setConnected(!m->isConnected(), true);
        }
    }
}

void ModuleControl::onLoop()
{
    for (IModule *m : _modules)
    {
        m->onLoop();
    }
}

int ModuleControl::moduleCount()
{
    return _modules.size();
}

bool ModuleControl::isModuleConnected(int i)
{
    return _modules[i]->isConnected();
}

void ModuleControl::beforeShutdown()
{
    for (IModule *m : _modules)
    {
        m->beforeShutdown();
    }
}

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
    if (pressed >= 0)
    {
        for (IModule *m : _modules)
        {
            if (m->getPosition() == pressed)
            {
                m->setConnected(!m->isConnected(), true);
            }
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
    if (_modules.empty())
        return false;

    for (IModule *m : _modules)
    {
        if (m->getPosition() == i)
        {
            return m->isConnected();
        }
    }
    return false;
}

void ModuleControl::beforeShutdown()
{
    for (IModule *m : _modules)
    {
        m->beforeShutdown();
    }
}

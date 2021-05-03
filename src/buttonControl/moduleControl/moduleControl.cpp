#include "moduleControl.h"

#include "../../modules/module.h"
#include "../../analog/analog.h"
#include "../../led/ledControl.h"

ModuleControl moduleControl;

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

    //Check bluetooth button

    //debounce and same button check
    if (millis() < DEBOUNCE_INTERVAL + last_pressed_time || last_pressed_button == pressed)
    {
        return;
    }

    printA("Pressed button: ");
    printlnA(pressed);

    last_pressed_button = pressed;
    last_pressed_time = millis();
    if (pressed != MODULE_BUTTON_RELEASED && pressed < _modules.size())
    {
        IModule *m = _modules[pressed];
        if (m != nullptr)
        {
            m->setConnected(!m->isConnected(), true);
        }
        ledControl.updateLedStatus();
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

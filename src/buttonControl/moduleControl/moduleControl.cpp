#include "moduleControl.h"

#include "../../modules/module.h"
#include "../../analog/analog.h"

ModuleControl moduleControl;

ModuleControl::ModuleControl()
{

    _modules.reserve(8);
    pinMode(MODULES_LATCH_PIN, OUTPUT);
    pinMode(MODULES_CLOCK_PIN, OUTPUT);
    pinMode(MODULES_DATA_PIN, OUTPUT);
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
        updateLedStatus();
    }
}

void ModuleControl::start()
{
    updateLedStatus();
}

void ModuleControl::onLoop()
{
    for (IModule *m : _modules)
    {
        m->onLoop();
    }
}

void ModuleControl::updateLedStatus()
{
    _ledByte = 0;
    for (int i = 0; i < _modules.size(); i++)
    {
        if (!_modules[i]->isConnected())
        {
            bitSet(_ledByte, i);
        }
    }

    for (int i = _modules.size(); i < 8; i++)
    {
        bitSet(_ledByte, i);
    }

    printlnA("Updating LED status");
    printlnA(_ledByte, 2);
    digitalWrite(MODULES_LATCH_PIN, LOW);
    shiftOut(MODULES_DATA_PIN, MODULES_CLOCK_PIN, MSBFIRST, _ledByte);
    digitalWrite(MODULES_LATCH_PIN, HIGH);
    printlnA("Updated");
}

void ModuleControl::beforeShutdown()
{
    for (IModule *m : _modules)
    {
        m->beforeShutdown();
    }
}

// void ModuleControl::setBrightness(int brightness)
// {
//     if (brightness > MAX_BRIGHTNESS)
//         brightness = MAX_BRIGHTNESS;
//     if (brightness < MIN_BRIGHTNESS)
//         brightness = MIN_BRIGHTNESS;
//     ledcWrite(BRIGHTNESS_CHANNEL, brightness);
// }
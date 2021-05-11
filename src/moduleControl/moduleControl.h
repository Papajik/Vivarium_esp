#ifndef _MODULE_CONTORL_H_
#define _MODULE_CONTROL_H_

#include <HardwareSerial.h>

//http://www.ignorantofthings.com/2018/07/the-perfect-multi-button-input-resistor.html
//http://www.openrtos.net/taskresumefromisr.html

#include <vector>
#define MODULE_COUNT 7

// #define MODULE_BUTTON_RELEASED -1

// #define DEBOUNCE_INTERVAL 200
#define M_BUTTONS_PIN 36

class IModule;

class ModuleControl
{
public:
    ModuleControl();
    /*
    @brief Adds module to module control
    @param Module
    @return Position in array
    */
    int addModule(IModule *);
    void start();
    /*
    @brief 
    @param 
    */
    void buttonPressed(int);

    void onLoop();

    int moduleCount();
    bool isModuleConnected(int);

    void beforeShutdown();

private:
    std::vector<IModule *> _modules;

    // int last_pressed_button = MODULE_BUTTON_RELEASED;
    // unsigned long last_pressed_time = 0;
};

#endif
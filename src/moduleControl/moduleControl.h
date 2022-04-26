#ifndef _MODULE_CONTORL_H_
#define _MODULE_CONTROL_H_

#include <HardwareSerial.h>
#include "../utils/classState/classState.h"

//http://www.ignorantofthings.com/2018/07/the-perfect-multi-button-input-resistor.html
//http://www.openrtos.net/taskresumefromisr.html

#include <vector>
#define MODULE_COUNT 7

#define M_BUTTONS_PIN 36

class IModule;

class ModuleControl: public ClassState
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
    /**
    * @brief Gets number of connected modules
    * 
    * @return int 
    */
    int moduleCount();

    /**
    * @brief Determine if module is connected or not
    * 
    * @return true 
    * @return false 
    */
    bool isModuleConnected(int);

    /**
    * @brief Calls method beforeShutdown on every module of the device.
    * 
    */
    void beforeShutdown();

private:
    std::vector<IModule *> _modules;

    // int last_pressed_button = MODULE_BUTTON_RELEASED;
    // unsigned long last_pressed_time = 0;
};

#endif
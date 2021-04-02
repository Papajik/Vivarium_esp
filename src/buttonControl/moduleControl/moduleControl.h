#ifndef _MODULE_CONTORL_H_
#define _MODULE_CONTROL_H_

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

//http://www.ignorantofthings.com/2018/07/the-perfect-multi-button-input-resistor.html
//http://www.openrtos.net/taskresumefromisr.html

#include <vector>
#define MODULE_COUNT 7

#define MODULE_BUTTON_RELEASED -1

#define DEBOUNCE_INTERVAL 200
#define M_BUTTONS_PIN 36

//Led diodes
#define MODULES_LATCH_PIN 18
#define MODULES_CLOCK_PIN 5
#define MODULES_DATA_PIN 15

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
    // void setBrightness(int brightness);
    void updateLedStatus();

    void beforeShutdown();

private:
    std::vector<IModule *> _modules;
    byte _ledByte = 0b00000000;

    int last_pressed_button = MODULE_BUTTON_RELEASED;
    unsigned long last_pressed_time = 0;
};

extern ModuleControl moduleControl;

#endif
/**
* @file buttonControl.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef _BUTTON_CONTROL_H_
#define _BUTTON_CONTROL_H_

class FirebaseService;
class BLEController;
class ModuleControl;

#define BUTTON_RELEASED -1

typedef void (*callback)(void);

/**
* @brief Controls button state.
* 
*/
class ButtonControl
{
public:
    ButtonControl(FirebaseService *, ModuleControl *);

    /**
    * @brief Debounce input and  notifies modules.
    * If bluetooth/Wi-Fi button is pushed, switch between those two services
    * 
    */
    void buttonPressed(int);

    /**
    * @brief Returns function callback for parallel task which checks button state
    * 
    * @return callback 
    */
    callback getCallback();

private:
    unsigned long _lastPressedTime = 0;
    int _lastPressedButton = BUTTON_RELEASED;

    FirebaseService *_firebaseService = nullptr;
    ModuleControl *_moduleControl = nullptr;
};

extern ButtonControl *buttonControl;

#endif
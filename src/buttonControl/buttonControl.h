#ifndef _BUTTON_CONTROL_H_
#define _BUTTON_CONTROL_H_

class FirebaseService;
class BLEController;
class ModuleControl;

#define BUTTON_RELEASED -1

class ButtonControl
{
public:
    ButtonControl(FirebaseService *, ModuleControl *);
    void buttonPressed(int);
    void start();

private:
    unsigned long _lastPressedTime = 0;
    int _lastPressedButton = BUTTON_RELEASED;

    FirebaseService *_firebaseService = nullptr;
    ModuleControl *_moduleControl = nullptr;
};

extern ButtonControl *buttonControl;

#endif
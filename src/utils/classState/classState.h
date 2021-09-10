#ifndef _CLASS_STATE_H
#define _CLASS_STATE_H

#include <Arduino.h>

class ClassState
{
public:
    ClassState(String name);
    unsigned long getLastMillis();
    String getStateString();
    int getStep();
    void printState();

protected:
    void setStep(int);
    void setStateString(String b);
    void setMillis();
    void setInnerState(ClassState *);

private:
    String _name = "";
    String _stateString = "";
    int _stateStep = -1;
    unsigned long _lastMillis = 0;
    ClassState *_innerState = nullptr;
};

#endif
/**
* @file classState.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/

#ifndef _CLASS_STATE_H
#define _CLASS_STATE_H

#include <Arduino.h>

/**
* @brief Tracks inner state of Class
* Debugger tool
* Used to determine where inheriting class froze 
*/
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
    unsigned long _previousMillis = 0;
    ClassState *_innerState = nullptr;
};

#endif
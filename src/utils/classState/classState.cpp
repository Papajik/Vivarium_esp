#include "classState.h"

ClassState::ClassState(String name) : _name(name) {}
unsigned long ClassState::getLastMillis()
{
    return _lastMillis;
}

String ClassState::getStateString()
{
    return _stateString;
}

int ClassState::getStep()
{
    return _stateStep;
}

void ClassState::setStep(int i)
{
    _stateStep = i;
}

void ClassState::setStateString(String b)
{
    _stateString = b;
}

void ClassState::setMillis()
{
    _previousMillis = _lastMillis;
    _lastMillis = millis();
}

void ClassState::setInnerState(ClassState *s)
{
    _innerState = s;
}

void ClassState::printState()
{
    Serial.printf("Class %s - step %d, info: %s millis: %lu (%lu) \n", _name.c_str(), _stateStep, _stateString.c_str(), _lastMillis, _previousMillis);
    if (_innerState != nullptr)
    {
        _innerState->printState();
    }
}
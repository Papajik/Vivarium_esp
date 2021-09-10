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
    _lastMillis = millis();
}
void ClassState::setInnerState(ClassState *s)
{
    _innerState = s;
}

void ClassState::printState()
{
    Serial.print("Class: ");
    Serial.print(_name);
    Serial.print(" - step: ");
    Serial.print(_stateStep);
    Serial.print(", info:");
    Serial.print(_stateString);
    Serial.print(", millis:");
    Serial.println(_lastMillis);
    if (_innerState != nullptr)
    {
        _innerState->printState();
    }
}
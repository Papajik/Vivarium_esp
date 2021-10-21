#include "HC_SR04.h"

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <vector>
#include <numeric>
HC_SR04 *sensor;

HC_SR04::HC_SR04(uint8_t trigger, uint8_t echo, int max_distance)
    : _triggerPin(trigger), _echoPin(echo), _max_distance(max_distance)
{
}

void HC_SR04::init()
{
    // printlnA("HC_SR04 begin");
    pinMode(_triggerPin, OUTPUT);
    digitalWrite(_triggerPin, LOW);
    pinMode(_echoPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_echoPin), _echo_isr, CHANGE);
}

void HC_SR04::startReadings()
{
    // printlnA("Start");
    _it = 0;
    _valid_it = 0;
    _state = SensorState::READING;
    startIteration();
}

void HC_SR04::stopReadings()
{
    //TODO
}

void HC_SR04::checkTimeout()
{
    if (micros() - _iterationStart > _max_distance * CM_ROUNDTRIP)
    {
        // printlnA("TIMEOUT");
        _readings[_it] = INVALID_VALUE;
        _it++;
        checkFinish();
    }
}

void HC_SR04::checkFinish()
{
    if (_it != ITERATION_COUNT)
    {
        startIteration();
    }
    else
    {
        _state = SensorState::FINISHED;
    }
}

void HC_SR04::finishIteration()
{
    _readings[_it] = (_end - _start) / CM_ROUNDTRIP;
    //
    if (_readings[_it] > _max_distance)
    {
        _readings[_it] = INVALID_VALUE;
    }
    else
    {
        _valid_it++;
    }
    // Serial.printf("Iteration Finished. S: %lu, E: %lu (%lu)\n", _start, _end, _readings[_it]);

    _it++;
    checkFinish();
}

void HC_SR04::update()
{

    if (_state == SensorState::READING)
    {
        if (_iterationFinished)
        {
            finishIteration();
        }
        else
        {
            checkTimeout();
        }
    }
}

void HC_SR04::startIteration()
{
    _iterationStart = micros();
    _start = 0;
    _end = 0;
    _iterationFinished = false;
    sendPulse();
}

void HC_SR04::sendPulse()
{
    digitalWrite(_triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_triggerPin, LOW);
}

bool HC_SR04::justFinished()
{
    if (_state == SensorState::FINISHED)
    {
        _state = SensorState::IDLE;
        return true;
    }
    else
    {
        return false;
    }
}

int HC_SR04::getRange()
{
    // Return error value on no valid reading
    if (_valid_it == 0)
        return INVALID_VALUE;

    std::vector<int> validReadings;

    for (int i = 0; i < ITERATION_COUNT; i++)
    {
        if (_readings[i] != INVALID_VALUE)
        {
            validReadings.push_back(_readings[i]);
        }
    }

    // Return the only valid readings
    if (validReadings.size() == 1)
        return validReadings.at(0);

    // Remove extremes
    if (validReadings.size() > 3)
    {
        std::sort(validReadings.begin(), validReadings.end());
        validReadings.erase(validReadings.begin());
        validReadings.pop_back();
    }
    return (std::accumulate(validReadings.begin(), validReadings.end(), 0.0) / (validReadings.size()) + 0.5);
}

void HC_SR04::_echo_isr()
{
    switch (digitalRead(sensor->_echoPin))
    {
    case HIGH:
        sensor->_start = micros();
        break;
    case LOW:
        sensor->_end = micros();
        sensor->_iterationFinished = true;
        break;
    }
}
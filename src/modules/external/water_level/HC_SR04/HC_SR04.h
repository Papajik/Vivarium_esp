#ifndef _V_HC_SR04_H
#define _V_HC_SR04_H

#include <Arduino.h>

#define MAX_SENSOR_DISTANCE 200
#define ITERATION_COUNT 10
#define CM_ROUNDTRIP 58.48
#define INVALID_VALUE -1

enum SensorState
{
    READING,
    IDLE,
    FINISHED,
};

class HC_SR04
{
public:
    HC_SR04(uint8_t trigger, uint8_t echo, int max_distance = MAX_SENSOR_DISTANCE);
    void init();
    void update();
    void startReadings();
    void stopReadings();
    bool justFinished();
    int getRange();
    SensorState getState() { return _state; }

private:
    SensorState _state = SensorState::IDLE;
    void sendPulse();
    void startIteration();
    void finishIteration();
    void checkTimeout();
    void checkFinish();

    bool _isReadRunning;
    bool _iterationFinished;
    int _it = 0;
    int _valid_it = 0;

    int _triggerPin, _echoPin, _max_distance;
    static void _echo_isr();
    unsigned long _readings[ITERATION_COUNT] = {};
    unsigned long _start, _end, _iterationStart;
};

extern HC_SR04 *sensor;

#endif
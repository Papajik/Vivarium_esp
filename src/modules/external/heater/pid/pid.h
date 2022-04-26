/**
* @file pid.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef HEATER_PID_H
#define HEATER_PID_H

#include <Arduino.h>

class QuickPID;
class MemoryProvider;

#define HEATER_DEFAULT_KP 80.0
#define HEATER_DEFAULT_KI 0.08
#define HEATER_DEFAULT_KD 0.8

/**
* @brief Used for tunings
* 
* @param inputVal 
* @return float 
*/
float pidAverage(float inputVal);

class HeaterPID
{
public:
    HeaterPID(MemoryProvider *provider);
    ~HeaterPID();
    void initPID(float *input, float *output, float *setpoint, float min, float max, float pb = 0);

    void saveOutputSum();
    void loadValues();

    void setStartTuning(bool b);
    void setTuneMode(uint8_t m) { _tuneMode = m; }

    void runPid();
    bool isAutoTuneRunning();

    void start();
    void stop();

    float getKp();
    float getKi();
    float getKd();
    float getPOn();
    uint8_t getTuneMode() { return _tuneMode; }

    void setKp(float kp, bool uploadChange = false);
    void setKd(float kd, bool uploadChange = false);
    void setKi(float ki, bool uploadChange = false);
    void setPOn(float POn, bool uploadChange = false);

    int getMode();

    bool isKiChanged() { return _ki_c; }
    bool isKdChanged() { return _kd_c; }
    bool isKpChanged() { return _kp_c; }
    bool isPonChanged() { return _pOn_c; }
    void clearChanges()
    {
        _ki_c = false;
        _kd_c = false;
        _kp_c = false;
        _pOn_c = false;
    }

private:
    void tuneLoop();
    void pidLoop();
    void startAutoTune();

    bool _pOn_c, _kp_c, _kd_c, _ki_c = false;

    // Automatic-Manual switch
    float _Pb; // Proportionality band
    float *_input, *_setpoint, *_output;

    MemoryProvider *_memoryProvider;
    uint8_t _tuneMode = 1; // Ziegler-Nichols PID

    QuickPID *_pid = nullptr;
    // float kp = HEATER_KP;
    // float ki = HEATER_KI;
    // float kd = HEATER_KD;

    float minOutput, maxOutput;

    bool _startTuning = false;
};

#endif
#include "pid.h"
#include "QuickPID.h"

#include "../../../../memory/memory_provider.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <Arduino.h>

#define HEATER_SAMPLE_TIME_US 1000000 * 5 // 5 seconds
#define HEATER_PON_KEY "pid_pon"
#define HEATER_KI_KEY "pid_ki"
#define HEATER_KP_KEY "pid_kp"
#define HEATER_KD_KEY "pid_kd"
#define PID_OUTPUT_MEMORY_KEY "pid_oSum"

float pidAverage(float inputVal)
{
    static float arrDat[16];
    static int pos;
    static float sum;
    pos++;
    if (pos >= 16)
        pos = 0;
    sum = sum - arrDat[pos] + inputVal;
    arrDat[pos] = inputVal;
    return (float)sum / 16.0;
}


HeaterPID::HeaterPID(MemoryProvider *p)
{
    _memoryProvider = p;
}

void HeaterPID::initPID(float *input, float *output, float *setpoint, float min, float max, float pb)
{
    printlnA("Init PID");
    _pid = new QuickPID(input, output, setpoint, HEATER_DEFAULT_KP, HEATER_DEFAULT_KI, HEATER_DEFAULT_KD, 0.4, 0, QuickPID::DIRECT); //PnOn 75 %
    _pid->SetSampleTimeUs(HEATER_SAMPLE_TIME_US);
    _pid->SetOutputLimits(min, max);
    minOutput = min;
    maxOutput = max;
    _Pb = pb;
    _input = input;
    _setpoint = setpoint;
    _output = output;
    // loadValues();
}

HeaterPID::~HeaterPID()
{
    if (_pid)
        delete _pid;
}

void HeaterPID::tuneLoop() { printlnA("Error"); }
void HeaterPID::pidLoop()
{
    if (_Pb != 0)
    {
        // If proportionality band is set, check whether Automatic or Manual control should be applied
        if (abs(*_input - *_setpoint) > _Pb)
        {
            // Use Manual when input is outside of proportinality band
            if (_pid->GetMode() == QuickPID::AUTOMATIC)
            {
                printlnA("Switching PID to manual");
                stop();
            }

            if (*_input > *_setpoint)
            {
                *_output = minOutput;
            }
            else
            {
                *_output = maxOutput;
            }
        }
        else // Use PID on close range
        {
            if (_pid->GetMode() == QuickPID::MANUAL)
            {

                if (*_input < *_setpoint) // set output to 0 so pid has chance to dumper the temperature increase
                {
                    *_output = minOutput;
                }
                printlnA("Swithich PID to Automatic");
                start();
            }
        }
    }
    _pid->Compute(); // Works only in automatic mode (it is ok to call it every time)
}

int HeaterPID::getMode()
{
    return _pid->GetMode();
}

void HeaterPID::startAutoTune()
{
    printlnA("Auto Tune Started");
    _startTuning = false;
    if (constrain(maxOutput / 3, minOutput, maxOutput - 0.1 - 5) < (maxOutput / 3)) //0.1 = output step
    {
        printlnE("AutoTune test exceeds outMax limit. Check output, hysteresis and outputStep values");
    }
    _pid->AutoTune(tuningMethod::ZIEGLER_NICHOLS_PID);
    _pid->autoTune->autoTuneConfig(0.1, 1, 25, 33, QuickPID::DIRECT, true, HEATER_SAMPLE_TIME_US);
}

bool HeaterPID::isAutoTuneRunning()
{
    return _pid->autoTune != nullptr;
}

void HeaterPID::runPid()
{

    if (_startTuning)
        startAutoTune();

    if (_pid->autoTune)
    {
        printlnD("tuneLoop");
        tuneLoop();
    }
    else
    {

        pidLoop();
    }
}

void HeaterPID::setStartTuning(bool b)
{
    printlnA("setStartTuning = " + b ? "true" : "false");
    _startTuning = b;
}

void HeaterPID::start()
{
    _pid->SetMode(QuickPID::AUTOMATIC);
}
void HeaterPID::stop()
{
    _pid->SetMode(QuickPID::MANUAL);
}

void HeaterPID::saveOutputSum()
{
    printlnA("Saving outputSum");
    printlnA(_pid->GetOutpoutSum());
    _memoryProvider->saveFloat(PID_OUTPUT_MEMORY_KEY, _pid->GetOutpoutSum());
}

void HeaterPID::loadValues()
{
    float i = _memoryProvider->loadFloat(HEATER_KI_KEY, HEATER_DEFAULT_KI);
    setKi(i, true);
    float p = _memoryProvider->loadFloat(HEATER_KD_KEY, HEATER_DEFAULT_KP);
    setKp(p, true);
    float d = _memoryProvider->loadFloat(HEATER_KP_KEY, HEATER_DEFAULT_KD);
    setKd(d, true);

    float sum = _memoryProvider->loadFloat(PID_OUTPUT_MEMORY_KEY, -1); // Load previous sum before restart
    if (sum != -1)
    {
        printlnA("Loading outputSum");
        printlnA(sum);
        *_output = sum;
        _pid->SetOutputSum(sum);
        _memoryProvider->removeKey(PID_OUTPUT_MEMORY_KEY);
    }
}

float HeaterPID::getKp() { return _pid->GetKp(); }
float HeaterPID::getKi() { return _pid->GetKi(); }
float HeaterPID::getKd() { return _pid->GetKd(); }
float HeaterPID::getPOn() { return _pid->GetPOn(); }

void HeaterPID::setKp(float kp, bool uploadChange)
{
    if (_pid->GetKp() != kp)
    {
        _pid->SetTunings(kp, _pid->GetKi(), _pid->GetKd());
        _memoryProvider->saveFloat(HEATER_KP_KEY, kp);
        if (uploadChange)
            _kp_c = true;
    }
}

void HeaterPID::setKd(float kd, bool uploadChange)
{
    if (_pid->GetKd() != kd)
    {
        _pid->SetTunings(_pid->GetKp(), _pid->GetKi(), kd);
        _memoryProvider->saveFloat(HEATER_KD_KEY, kd);
        if (uploadChange)
            _kd_c = true;
    }
}

void HeaterPID::setKi(float ki, bool uploadChange)
{
    if (_pid->GetKi() != ki)
    {
        _pid->SetTunings(_pid->GetKp(), ki, _pid->GetKd());
        _memoryProvider->saveFloat(HEATER_KI_KEY, ki);
        if (uploadChange)
            _ki_c = true;
    }
}

void HeaterPID::setPOn(float pOn, bool uploadChange)
{
    if (_pid->GetPOn() != pOn)
    {
        _pid->SetTunings(_pid->GetKp(), _pid->GetKi(), _pid->GetKd(), pOn, 0);
        _memoryProvider->saveFloat(HEATER_PON_KEY, pOn);
        if (uploadChange)
            _pOn_c = true;
    }
}


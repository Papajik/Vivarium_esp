#include "heater.h"
#include "../../../state/state_values.h"
#include <RBDdimmer.h> //
#include <AutoPID.h>   //https://r-downing.github.io/AutoPID/
#include "millisDelay.h"
#include "state_values.h"

#define CONNECTED_KEY "heater/c"
#define FIREBASE_CD_POWER "/sensorData/heater/power"
#define FIREBASE_CD_TEMP_GOAL "/sensorData/heater/tempGoal"
#define INTEGRAL_KEY "heat_t"

#define HEATER_OUTPUT_MIN 0
#define HEATER_OUTPUT_MAX 100

#define HEATER_STEP_TIME 5000
#define HEATER_BANG_BANG 1

#define HEATER_FAILSAFE_DELAY 10000

#define HEATER_TEMP_INVALID -127

String modeToString(Mode m)
{
    switch (m)
    {
    case PID:
        return "PID";
    case AUTO:
        return "AUTO";
    case THERMO:
        return "THERMO";
    default:
        return "UNKNOWN";
    }
}

Heater::Heater(int position, int pwm, int sync) : IModule(CONNECTED_KEY, position)
{
    printlnA("Heater created");

    _settings = {AUTO, GOAL_INVALID};

    _dimmer = new dimmerLamp(pwm, sync);
    _dimmer->begin(NORMAL_MODE, OFF);
    _pid = new AutoPID(&_currentTemperature, &_settings.tempGoal, &_currentPower, HEATER_OUTPUT_MIN, HEATER_OUTPUT_MAX, HEATER_KP, HEATER_KI, HEATER_KD);
    _pid->setBangBang(HEATER_BANG_BANG);
    _pid->setTimeStep(HEATER_STEP_TIME);
    setGoal(_settings.tempGoal);
}

void Heater::beforeShutdown()
{
    // printlnA("Heater - shutting down saving integral");
    // _memoryProvider->saveDouble(INTEGRAL_KEY, _pid->getIntegral());
}

void Heater::onLoop()
{
    if (_settingsChanged)
        saveSettings();

    checkConnectionChange();

    if (isConnected())
    {
        //Failsafe
        if (!checkTemperatureConnected())
            return;

        if (_settings.mode == PID)
        {
            runPID();
        }

        if (_settings.mode == THERMO)
        {
            runThermo();
        }
    }
}

bool Heater::loadTemp()
{
    float tmp;
    if (stateStorage.getValue(STATE_WATER_TEMPERATURE, &tmp))
    {
        if (tmp != HEATER_TEMP_INVALID)
        {

            _lastValidTemp = millis();
            _currentTemperature = tmp;
            printD("CurrentTemp = ");
            printlnD(_currentTemperature);
            return true;
        }
        else
        {
            printlnA("Water temp is invalid");
            return failSafeCheck();
        }
    }
    else
    {
        printlnA("Reading water temp failed");
        return false;
    }
}

void Heater::runThermo()
{
    if (loadTemp())
    {
        if (_currentTemperature >= _settings.tempGoal)
        {
            _currentPower = 0;
        }
        else
        {
            _currentPower = 100;
        }
        setPower();
    }
}

bool Heater::checkTemperatureConnected()
{
    bool tempConnected;
    stateStorage.getValue(STATE_WATER_TEMP_CONNECTED, &tempConnected);
    if (!tempConnected)
    {
        setConnected(false, true);
        return false;
    }
    return true;
}

void Heater::stop()
{
    _currentPower = 0;
    setPower();
}

void Heater::runPID()
{
    if (loadTemp())
    {
        _pid->run();
        setPower();
    }
}
void Heater::setPower()
{
    _dimmer->setPower(_currentPower);
    stateStorage.setValue(HEATER_POWER, (float)_currentPower);

    if (isBluetoothRunning())
    {
        _currentPowerCharacteristic->setValue(_currentPower);
        _currentPowerCharacteristic->notify();
    }

    if (_oldPower != _currentPower)
    {
        _oldPower = _currentPower;
        printlnD("Uploading new temperature");
        firebaseService->uploadCustomData("devices/", FIREBASE_CD_POWER, _currentPower);
    }
}

void Heater::saveSettings()
{
    _memoryProvider->saveStruct(SETTINGS_HEATER_KEY, &_settings, sizeof(HeaterSettings));
    _settingsChanged = false;
}

bool Heater::loadSettings()
{

    /// Check if integral is saved and set it to pid regulator if so
    double integral = _memoryProvider->loadDouble(INTEGRAL_KEY, -1);

    if (integral != -1)
    {
        printlnA("Loaded integral = " + String(integral));
        _pid->setIntegral(integral);
        _memoryProvider->removeKey(INTEGRAL_KEY);
    }

    /// Load settings struct
    bool loaded = _memoryProvider->loadStruct(SETTINGS_HEATER_KEY, &_settings, sizeof(HeaterSettings));

    if (loaded)
    {
        setGoal(_settings.tempGoal);
    }
    return loaded;
}

void Heater::onConnectionChange()
{
    if (!isConnected())
    {
        printlnA("HEATER - disconnected");
        _dimmer->setState(OFF);
    }
    else
    {
        printlnA("HEATER - connected");
        _dimmer->setState(ON);
        _lastValidTemp = millis();
    }

    sendConnectionChangeNotification("Heater", isConnected());

    if (_sourceIsButton)
    {
        firebaseService->uploadState(FIREBASE_HEATER_CONNECTED_KEY, isConnected());
        _sourceIsButton = false;
    }

    if (isBluetoothRunning())
    {
        std::string s = isConnected() ? "true" : "false";
        _connectedCharacteristic->setValue(s);
        _connectedCharacteristic->notify();
    }
}

void Heater::setMode(Mode m)
{
    if (m != _settings.mode)
    {
        stateStorage.setValue(HEATER_MODE, (uint32_t)m);
        _settings.mode = m;
        _settingsChanged = true;
        _pid->reset();
    }
}

void Heater::setGoal(double g)
{
    if (g != _settings.tempGoal)
    {
        printlnA("Heater - set goal");
        printA("goal = ");
        printlnA(_settings.tempGoal);
        //TODO check this one
        _settings.tempGoal = g;
        _settingsChanged = true;
        stateStorage.setValue(TEMP_GOAL, (float)g);
        _pid->reset();
        firebaseService->uploadCustomData("devices/", FIREBASE_CD_TEMP_GOAL, g);
    }
}

double Heater::getCurrentPower()
{
    return _currentPower;
}

double Heater::getGoal()
{
    return _settings.tempGoal;
}

bool Heater::failSafeCheck()
{
    if (millis() > _lastValidTemp + HEATER_FAILSAFE_DELAY)
    {
        printlnA("Heater disconnected");
        printlnE("Heater failsafe");
        stop();
        setConnected(false, true);
        return false;
    }
    return true;
}

std::vector<String> Heater::getText()
{
    if (!_connected)
    {
        return {"Heater", "Disconnected"};
    }
    else
    {
        String secondRow = "";
        if (_settings.mode == Mode::PID || _settings.mode == Mode::THERMO)
        {
            secondRow = "G: " + String(_settings.tempGoal, 1) + ", PWR: " + String(_currentPower, 1);
        }
        return {"Heater: " + modeToString(_settings.mode), secondRow};
    }
}
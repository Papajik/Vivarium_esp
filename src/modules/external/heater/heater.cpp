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

Heater::Heater() : IModule(CONNECTED_KEY)
{
    printlnA("Heater created");

    if (!loadSettings())
    {
        _settings = {AUTO, GOAL_INVALID};
    }

    _dimmer = new dimmerLamp(HEATER_PIN, HEATER_SYNC_PIN);
    _dimmer->begin(NORMAL_MODE, OFF);
    _pid = new AutoPID(&_currentTemperature, &_settings.tempGoal, &_currentPower, HEATER_OUTPUT_MIN, HEATER_OUTPUT_MAX, HEATER_KP, HEATER_KI, HEATER_KD);
    _pid->setBangBang(3);
    _pid->setTimeStep(5000);

    double integral = memoryProvider.loadDouble(INTEGRAL_KEY, -1);

    if (integral != -1)
    {
        _pid->setIntegral(integral);
        memoryProvider.removeKey(INTEGRAL_KEY);
    }

    setGoal(_settings.tempGoal);
}

void Heater::beforeShutdown()
{
    printlnA("Heater - shutting down saving integral");
    memoryProvider.saveDouble(INTEGRAL_KEY, _pid->getIntegral());
}

void Heater::onLoop()
{
    if (_settingsChanged)
        saveSettings();

    checkConnectionChange();

    if (isConnected())
    {
        if (_settings.mode == PID)
        {
            runPID();
        }
    }
}

void Heater::runPID()
{
    //Failsafe
    bool tempConnected;
    stateStorage.getValue(STATE_WATER_TEMP_CONNECTED, &tempConnected);
    if (!tempConnected)
    {
        setConnected(false, true);
        return;
    }

    //update temp
    float tmp;
    if (stateStorage.getValue(STATE_WATER_TEMPERATURE, &tmp))
    {
        if (tmp != HEATER_TEMP_INVALID)
        {

            _lastValidTemp = millis();
            _currentTemperature = tmp;
            printD("CurrentTemp = ");
            printlnD(_currentTemperature);
        }
        else
        {
            printlnA("Water temp is invalid");
            failSafeCheck();
        }
    }
    else
    {
        printlnA("Reading water temp failed");
    }

    _pid->run();
    printD("At setpoint =");
    printlnD(_pid->atSetPoint(0.5));
    printD("Power = ");
    printlnD(_currentPower);
    printD("Goal = ");
    printlnD(_settings.tempGoal);
    _pid->printSettings();
    setPower();
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
        firebaseService.uploadCustomData("devices/", FIREBASE_CD_POWER, _currentPower);
    }
}

void Heater::saveSettings()
{
    memoryProvider.saveStruct(SETTINGS_HEATER_KEY, &_settings, sizeof(HeaterSettings));
    _settingsChanged = false;
}
bool Heater::loadSettings()
{
    return memoryProvider.loadStruct(SETTINGS_HEATER_KEY, &_settings, sizeof(HeaterSettings));
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
        firebaseService.uploadState(FIREBASE_HEATER_CONNECTED_KEY, isConnected());
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
        firebaseService.uploadCustomData("devices/", FIREBASE_CD_TEMP_GOAL, g);
    }

    // if (g != _tempGoal)
    // {
    //     _tempGoal = g;
    //     _settings.tempGoal = g;
    //     _settingsChanged = true;
    //     stateStorage.setValue(TEMP_GOAL, (float)g);
    //     _pid->reset();
    // }
}

double Heater::getCurrentPower()
{
    return _currentPower;
}

double Heater::getGoal()
{
    return _settings.tempGoal;
}

void Heater::failSafeCheck()
{
    if (millis() > _lastValidTemp + HEATER_FAILSAFE_DELAY)
    {
        printlnA("Heater disconnected");
        printlnE("Heater failsafe");
        setConnected(false, true);
    }
}
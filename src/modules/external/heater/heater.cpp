#include "heater.h"
#include <Arduino.h>
#include "../../../state/state_values.h"
#include "pid/pid.h"
#include <RBDdimmer.h> 
#include "millisDelay.h"
#include "state_values.h"
#include "../../../wifi/wifiProvider.h"
#include "../../../utils/rtc/rtc.h"
#define CONNECTED_KEY "heater/c"
#define FIREBASE_CD_POWER "/sensorData/heater/power"
#define FIREBASE_CD_TEMP_GOAL "/sensorData/heater/goal"

#define HEATER_THERMO_CYCLE 15 * 1000 // 30 second (cca 1/4 of dead time cycle - 2-3 minutes)

#define HEATER_OUTPUT_MIN 0
#define HEATER_OUTPUT_MAX 100

#define HEATER_STEP_TIME 5000
#define HEATER_BANG_BANG 1

#define HEATER_SAMPLE_TIME_US 1000000 * 5 // 5 seconds

#define HEATER_FAILSAFE_DELAY 10000

#define HEATER_TEMP_INVALID -127

Heater *heaterPtr = nullptr;

void heaterCallback()
{
    if (heaterPtr != nullptr)
    {
        heaterPtr->triggerCallback();
    }
    else
    {
        printlnE("Led Module is nullptr");
    }
}

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
    case DIRECT:
        return "DIRECT";
    default:
        return "UNKNOWN";
    }
}

void Heater::setFutureGoal(double goal)
{
    _futureGoal = goal;
}

Heater::Heater(int position, MemoryProvider *provider, int pwm, int sync)
    : IModule(CONNECTED_KEY, position, provider),
      PayloadAlarm<double>(&heaterCallback, provider, "heat.")

{
    printlnA("Heater created");
    _settings = {AUTO, GOAL_INVALID, 0};
    _thermoDelay = new millisDelay();
    _pid = new HeaterPID(provider);
    _pid->initPID(&_currentTemperature, &_currentPower, &_settings.tempGoal, HEATER_OUTPUT_MIN, HEATER_OUTPUT_MAX, 0.5); // bang-bang 0.5 °C
    _dimmer = new dimmerLamp(pwm, sync);
    _dimmer->begin(NORMAL_MODE, OFF);
    loadSettings();
}

void Heater::beforeShutdown()
{
    if (getMode() == Mode::PID)
    {
        _pid->saveOutputSum();
    }
}

void Heater::checkFutureGoal()
{
    if (_futureGoal != -1)
    {

        printlnA("Future goal changed");
        printlnA(_futureGoal);
        setGoal(_futureGoal);
        _settings.tempGoal = _futureGoal;
        _settingsChanged = true;

        if (isBluetoothRunning())
        {

            _currentGoalCharacteristic->setValue(_futureGoal);
            _currentGoalCharacteristic->notify();
        }
        if (firebaseService != nullptr)
            firebaseService->uploadState(KEY_HEATER_GOAL, (float)_futureGoal);

        if (messagingService != nullptr)
            messagingService->sendFCM("Heater", "Heater triggered! Current goal= " + String(_futureGoal, 2) + "°C", FCM_TYPE::TRIGGER, SETTINGS_HEATER_KEY);
        _futureGoal = -1;
    }
}

void Heater::onLoop()
{
    checkFutureGoal();
    if (_settingsChanged)
        saveSettings();

    /// PID values change
    if (_pid->isKiChanged())
        firebaseService->uploadState(KEY_HEATER_KI, _pid->getKi());
    if (_pid->isKpChanged())
        firebaseService->uploadState(KEY_HEATER_KP, _pid->getKp());
    if (_pid->isKdChanged())
        firebaseService->uploadState(KEY_HEATER_KD, _pid->getKd());
    if (_pid->isPonChanged())
        firebaseService->uploadState(KEY_HEATER_PON, _pid->getPOn());
    _pid->clearChanges();

    checkConnectionChange();

    if (isConnected())
    {
        if (_settings.mode == Mode::DIRECT) // Direct output is set explicitely by user and there is no temperature input needed
        {
            _currentPower = _settings.directPower;
            updatePower();
            return;
        }

        //Failsafe
        if (!checkTemperatureConnected())
            return;

        //Temperature loaded
        if (loadTemp())
        {
            // PID
            if (_settings.mode == Mode::PID)
            {

                _pid->runPid();
                updatePower();
            }

            // Thermo
            if (_settings.mode == Mode::THERMO)
            {
                runThermo();
                updatePower();
            }
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
            if (_pid->isAutoTuneRunning()) // only if pid exists and tune is running
            {
                // User running (rolling) average to get smooth output
                _currentTemperature = pidAverage(tmp);
            }
            else
            {
                _currentTemperature = tmp;
            }

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
    // Check if cycle is running, if not, start cycle
    if (!_thermoDelay->isRunning())
    {
        _thermoDelay->start(HEATER_THERMO_CYCLE);
        printlnA("Thermo is not running, starting now");
    }

    if (_thermoDelay->justFinished())
    {
        printlnA("Thermo finished") if (_currentTemperature >= _settings.tempGoal)
        {
            _currentPower = 0;
        }
        else
        {
            _currentPower = 100;
        }
        _thermoDelay->restart();
    }
}

bool Heater::checkTemperatureConnected()
{
    bool tempConnected;
    stateStorage.getValue(STATE_WATER_TEMP_CONNECTED, &tempConnected);
    if (!tempConnected)
    {
        printlnW("Temp is not connected, disconnecting heater");
        setConnected(false, true);
        return false;
    }
    return true;
}

void Heater::stop()
{
    _currentPower = 0;
    updatePower();
}

void Heater::updatePower()
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
        debugA("Heater: New power = %f, uploading data", _currentPower);
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

    /// Load settings struct
    bool loaded = _memoryProvider->loadStruct(SETTINGS_HEATER_KEY, &_settings, sizeof(HeaterSettings));

    if (loaded)
    {
        setGoal(_settings.tempGoal, true);
        setMode(_settings.mode);
    }
    _pid->loadValues();
    //Check whether Wi-Fi or RTC is running to ensure time is valid
    if (wifiProvider->isConnected() || rtc.isRunning())
    {
        loadTriggersFromNVS();
        printTriggers();
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

        // setup pid
        if (m == Mode::PID)
        {
            _pid->start();
        }
        else
        {
            _pid->stop();
        }

        // setup thermo
        if (m == Mode::THERMO)
        {
            printlnA("Starting Thermo cycle");
            _thermoDelay->start(HEATER_THERMO_CYCLE); // Set dead time cycle
        }
        else
        {
            printlnA("Stoping Thermo cycle");
            _thermoDelay->stop(); // stop cycle
        }
    }
}

void Heater::setGoal(double g, bool forced)
{
    printlnA("Heater -  setGoal");
    if (g != _settings.tempGoal || forced)
    {
        printlnA("Goal changed");
        debugA("New goal = %.2f", g);
        printlnA();
        _settings.tempGoal = g;
        _settingsChanged = true;
        stateStorage.setValue(TEMP_GOAL, (float)g);
        if (firebaseService != nullptr)
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

void Heater::printPidSettings()
{
    debugA("PID mode = %d", _pid->getMode());
    debugA("Kp = %.2f", _pid->getKp());
    debugA("Ki = %.2f", _pid->getKi());
    debugA("Kd = %.2f", _pid->getKd());
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

void Heater::triggerCallback()
{
    AlarmId id = Alarm.getTriggeredAlarmId();
    double goal;
    if (PayloadAlarm<double>::getTriggerPayload(id, &goal))
    {
        printA("Heater goal: ");
        printlnA(String(goal));
        setFutureGoal(goal);
    }
}
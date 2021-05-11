#include "water_level.h"
#include <NewPing.h> //https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home
#include <millisDelay.h>
#include "state_values.h"

#define CONNECTED_KEY "wl/c"
#define FIREBASE_CD_LEVEL "/sensorData/wl/level"
#define FCM_KEY "Water Level"

#define W_LEVEL_REPEAT_AFTER 2000

WaterLevel::WaterLevel(int position, int echo, int trig) : IModule(CONNECTED_KEY, position)
{
    printlnA("Water level created");
    _delay = new millisDelay();
    _sonar = new NewPing(trig, echo);
    _delay->start(W_LEVEL_REPEAT_AFTER);
    _settings = {0, 0, 0};
}
void WaterLevel::setMaxLevel(int l)
{
    if (_settings.maxLevel != l)
    {
        _settings.maxLevel = l;
        _settingsChanged = true;
    }
}
void WaterLevel::setMinLevel(int l)
{
    if (_settings.minLevel != l)
    {
        _settings.minLevel = l;
        _settingsChanged = true;
    }
}

void WaterLevel::setSensorHeight(int h)
{
    if (_settings.sensorHeight != h)
    {
        _settings.sensorHeight = h;
        _settingsChanged = true;
    }
}

int WaterLevel::getWaterLevel()
{
    return _waterLevel;
}

int WaterLevel::getMinLevel()
{
    return _settings.minLevel;
}

int WaterLevel::getMaxLevel()
{
    return _settings.maxLevel;
}

int WaterLevel::getSensorHeight()
{
    return _settings.sensorHeight;
}

void WaterLevel::onLoop()
{
    checkConnectionChange();
    if (_settingsChanged)
    {
        saveSettings();
    }
    if (isConnected())
    {
        if (_delay->justFinished())
        {

            readLevel();
            _delay->restart();
        }
    }
}

void WaterLevel::readLevel()
{

    unsigned int _echoTime = _sonar->ping_median(7);
    unsigned int _distance = _sonar->convert_cm(_echoTime);
    printlnV("Distance = " + String(_distance));
    int level = _settings.sensorHeight - _distance;
    setLevel(level);
}

void WaterLevel::setLevel(int level)
{
    if (_waterLevel != level)
    {
        _waterLevel = level;
        stateStorage.setValue(STATE_WATER_LEVEL, (uint32_t)_waterLevel);
        if (isBluetoothRunning())
        {
            _waterLevelCharacteristic->setValue(_waterLevel);
            _waterLevelCharacteristic->notify();
        }
        firebaseService->uploadCustomData("devices/", FIREBASE_CD_LEVEL, _waterLevel);
    }
}

void WaterLevel::saveSettings()
{
    _memoryProvider->saveStruct(SETTINGS_WL_KEY, &_settings, sizeof(WaterLevelSettings));
    _settingsChanged = false;
}

bool WaterLevel::loadSettings()
{
    return _memoryProvider->loadStruct(SETTINGS_WL_KEY, &_settings, sizeof(WaterLevelSettings));
}

void WaterLevel::onConnectionChange()
{
    if (_sourceIsButton)
    {
        firebaseService->uploadState(FIREBASE_WL_CONNECTED_KEY, isConnected());
        _sourceIsButton = false;
    }
    stateStorage.setValue(STATE_WATER_LEVEL_CONNECTED, isConnected());

    sendConnectionChangeNotification(FCM_KEY, isConnected());

    if (isBluetoothRunning())
    {
        std::string s = isConnected() ? "true" : "false";
        _connectedCharacteristic->setValue(s);
        _connectedCharacteristic->notify();
    }
}

void WaterLevel::checkBoundaries()
{
    if (_waterLevel > _settings.maxLevel)
    {
        messagingService->sendFCM(FCM_KEY, "Water level is over maximum alowed value", FCM_TYPE::CROSS_LIMIT, FCM_KEY);
    }

    if (_waterLevel < _settings.minLevel)
    {
        messagingService->sendFCM(FCM_KEY, "Water level is below maximum alowed value", FCM_TYPE::CROSS_LIMIT, FCM_KEY);
    }
}
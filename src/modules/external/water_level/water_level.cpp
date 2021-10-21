#include "water_level.h"
// #include <NewPing.h> //https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home
#include <millisDelay.h>
#include "state_values.h"

#define CONNECTED_KEY "wl/c"
#define FIREBASE_CD_LEVEL "/sensorData/wl/level"
#define FCM_KEY "Water Level"

#define W_LEVEL_REPEAT_AFTER 700
#define W_LEVEL_DIFF_COUNT_MAX 7

#include "HC_SR04/HC_SR04.h"
#include "../../../utils/compare.h"

WaterLevel::WaterLevel(int position, MemoryProvider *m, int echo, int trig)
    : IModule(CONNECTED_KEY, position, m),
      _delay(new millisDelay())
{
    if (!loadSettings())
    {
        _settings = {0, 0, 0};
    }
    sensor = new HC_SR04(trig, echo);
    sensor->init();
    sensor->startReadings();
    printlnA("Water level created");
    _delay->start(W_LEVEL_REPEAT_AFTER);
}

WaterLevel::~WaterLevel()
{
    delete _delay;
    delete sensor;
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
    setMillis();
    setStep(0);
    checkConnectionChange();
    if (_settingsChanged)
    {
        Serial.println("WL - settings change  - saving");
        saveSettings();
        Serial.println("WL - settings change  - saved");
    }

    setStep(1);
    if (isConnected())
    {
        sensor->update();
        if (sensor->justFinished())
        {
            setStep(2);
            readLevel();
        }
        if (_delay->justFinished())
        {
            setStep(3);
            sensor->startReadings();
            _delay->restart();
        }
    }
    setStep(4);
}

void WaterLevel::readLevel()
{
    int range = sensor->getRange();
    if (range == INVALID_VALUE)
    {
        setLevel(INVALID_VALUE);
    }
    else
    {
        setLevel(_settings.sensorHeight - range);
    }
}

void WaterLevel::setLevel(int level)
{

    if (_waterLevel != level)
    {
        _differentWaterLevelCount++;

        if (_differentWaterLevelCount > W_LEVEL_DIFF_COUNT_MAX)
        {
            _differentWaterLevelCount = 0;
            _waterLevel = level;
            stateStorage.setValue(STATE_WATER_LEVEL, (uint32_t)_waterLevel);
            if (isBluetoothRunning())
            {
                _waterLevelCharacteristic->setValue(_waterLevel);
                _waterLevelCharacteristic->notify();
            }
            firebaseService->uploadCustomData("devices/", FIREBASE_CD_LEVEL, _waterLevel);
            checkBoundaries();
        }
    }
    else
    {
        _differentWaterLevelCount = 0;
    }
}

void WaterLevel::saveSettings()
{
    Serial.println("WL - saving settings");
    Serial.println(_settings.maxLevel);
    Serial.println(_settings.minLevel);
    Serial.println(_settings.sensorHeight);
    _memoryProvider->saveStruct(SETTINGS_WL_KEY, &_settings, sizeof(WaterLevelSettings));
    _settingsChanged = false;
}

bool WaterLevel::loadSettings()
{
    return _memoryProvider->loadStruct(SETTINGS_WL_KEY, &_settings, sizeof(WaterLevelSettings));
}

void WaterLevel::onConnectionChange()
{
    if (isConnected())
    {
        sensor->startReadings();
    }
    else
    {
        sensor->stopReadings();
    }

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
    if (messagingService == nullptr)
        return;

    if (_waterLevel > _settings.maxLevel)
    {
        char buffer[70];
        sprintf(buffer, "Water level (%d cm) is over maximum alowed value (%d cm)", _waterLevel, _settings.maxLevel);
        messagingService->sendFCM(FCM_KEY, String(buffer), FCM_TYPE::CROSS_LIMIT, String(FCM_KEY) + "l");
        _lastMessage = 1;
        _messageCount = 0;
        return;
    }

    if (_waterLevel < _settings.minLevel)
    {
        char buffer[70];
        sprintf(buffer, "Water level (%d cm) is below minimum alowed value (%d cm)", _waterLevel, _settings.minLevel);
        messagingService->sendFCM(FCM_KEY, String(buffer), FCM_TYPE::CROSS_LIMIT, String(FCM_KEY) + "l");
        _lastMessage = -1;
        _messageCount = 0;
        return;
    }

    /// If boundaries are ok:
    if (_lastMessage != 0 || (_lastMessage == 0 && _messageCount < 10))
    {
        char buffer[50];
        sprintf(buffer, "Water level (%d cm) is OK", _waterLevel);
        messagingService->sendFCM(FCM_KEY, String(buffer), FCM_TYPE::CROSS_LIMIT, String(FCM_KEY) + "l", true);
        _lastMessage = 0;
        _messageCount++;
    }
}

std::vector<String> WaterLevel::getText()
{
    if (!_connected)
    {
        return {"Water Level", "Disconnected"};
    }
    else
    {
        printlnD("WL - getText()");
        printlnD(_waterLevel);
        printlnD(_settings.minLevel);
        return {"Water Level: " + String(_waterLevel), "LL: " + String(_settings.minLevel) + " HL: " + String(_settings.maxLevel)};
    }
}
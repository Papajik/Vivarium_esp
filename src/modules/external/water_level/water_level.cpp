#include "water_level.h"
#include <NewPing.h> //https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home
#include <millisDelay.h>
#include "state_values.h"

#define CONNECTED_KEY "wl/c"
#define FIREBASE_CD_LEVEL "/sensorData/wl/level"
#define FCM_KEY "Water Level"

#define W_LEVEL_REPEAT_AFTER 3000
#define W_LEVEL_DELAY_CHANGE 10000
#define MAX_CM_DISTNCE 80

#include "../../../utils/compare.h"

WaterLevel::WaterLevel(int position, int echo, int trig)
    : IModule(CONNECTED_KEY, position),
      _delay(new millisDelay()),
      _sonar(new NewPing(trig, echo, MAX_CM_DISTNCE))
{
    printlnA("Water level created");
    _delay->start(W_LEVEL_REPEAT_AFTER);
    _settings = {0, 0, 0};
}

WaterLevel::~WaterLevel()
{
    delete _delay;
    delete _sonar;
}

void WaterLevel::setMaxLevel(int l)
{
    Serial.println("Set max level");
    Serial.println(l);
    if (_settings.maxLevel != l)
    {
        _settings.maxLevel = l;
        _settingsChanged = true;
    }
}
void WaterLevel::setMinLevel(int l)
{
    Serial.println("Set min level");
    Serial.println(l);
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
        Serial.println("WL - settings change  - saving");
        saveSettings();
        Serial.println("WL - settings change  - saved");
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
    unsigned int _echoTime;
    unsigned int _distance;
    for (int i = 0; i < READ_COUNT; i++)
    {
        _echoTime = _sonar->ping_median(10);
        _distance = _sonar->convert_cm(_echoTime);
        _levelBuffer[i] = _settings.sensorHeight - _distance;
    }

    qsort(_levelBuffer, READ_COUNT, sizeof(float), compareInt);

    int average = 0;
    for (int i = READ_CUT; i < READ_COUNT - READ_CUT; i++)
    {
        average += _levelBuffer[i];
    }

    setLevel(average / (READ_COUNT - (2 * READ_CUT)));
}

void WaterLevel::setLevel(int level)
{

    if (_waterLevel != level)
    {
        if (_lastValueChange + W_LEVEL_DELAY_CHANGE < millis())
        {
            _lastValueChange = millis();
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
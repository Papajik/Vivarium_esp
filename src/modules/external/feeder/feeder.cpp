#include "feeder.h"
#include "../../../wifi/wifiProvider.h"
#include "../../../utils/timeHelper.h"
#include "../../../utils/rtc/rtc.h"

#include <Stepper.h>

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#define CONNECTED_KEY "feeder/c"

void feederCallback()
{
    feederPtr->feed();
}

Feeder *feederPtr = nullptr;
Feeder::Feeder(int position, MemoryProvider *provider, int in_1, int in_2, int in_3, int in_4)
    : IModule(CONNECTED_KEY, position, provider), PlainAlarm(&feederCallback, provider, "feed.")
{
    _stepper = std::make_shared<Stepper>(FEEDER_STEPS_PER_REVOLUTION, in_1, in_3, in_2, in_4);
    loadSettings();
}

Feeder::~Feeder()
{
}
void Feeder::onLoop()
{
    checkConnectionChange();
    if (_settingsChanged)
    {
        saveSettings();
    }

    if (_feeded)
    {
        time_t now;
        time(&now);
        char timeArray[40];
        ultoa(now, timeArray, 10);

        firebaseService->uploadCustomData("feed/", "/" + std::string(timeArray), "true");

        _feeded = false;
    }
}
void Feeder::saveSettings()
{
    if (_memoryProvider != nullptr)
    {
        _memoryProvider->saveStruct(SETTINGS_FEEDER_KEY, &_settings, sizeof(FeederSettings));
    }
    _settingsChanged = false;
}

bool Feeder::loadSettings()
{
    if (_memoryProvider != nullptr && !_memoryProvider->loadStruct(SETTINGS_FEEDER_KEY, &_settings, sizeof(FeederSettings)))
    {
        _settings = {BOX};
        saveSettings();
    }

    //Check whether Wi-Fi or RTC is running to ensure time is valid
    if (wifiProvider->isConnected() || rtc.isRunning())
    {
        loadTriggersFromNVS();
        printTriggers();
    }

    return true;
}

FeederMode Feeder::getMode()
{
    return _settings.mode;
}

void Feeder::setMode(FeederMode m)
{
    if (m != _settings.mode)
    {
        _settings.mode = m;
        _settingsChanged = true;
    }
}

void Feeder::feed()
{
    if (!isConnected())
        return;

    int speed;
    int steps;
    switch (getMode())
    {
    case BOX:
        speed = 15;
        steps = 150;
        break;
    case SCREW:
        speed = 15;
        steps = 200;
        break;
    default:
        speed = 15;
        steps = 0;
        break;
    }

    _stepper->setSpeed(speed);
    _stepper->step(steps);
    digitalWrite(FEEDER_IN_1, LOW);
    digitalWrite(FEEDER_IN_2, LOW);
    digitalWrite(FEEDER_IN_3, LOW);
    digitalWrite(FEEDER_IN_4, LOW);

    _feeded = true;

    tm timeinfo;
    if (getLocalTime(&timeinfo, 100))
    {
        _lastFeededTime = getTime(timeinfo.tm_hour, timeinfo.tm_min);
    }
    if (messagingService != nullptr)
        messagingService->sendFCM("Feeder", "Feeder triggered", FCM_TYPE::TRIGGER, SETTINGS_FEEDER_KEY);
}

int Feeder::getLastFeeded()
{
    return _lastFeededTime;
}

void Feeder::onConnectionChange()
{

    if (_sourceIsButton)
    {
        firebaseService->uploadState(FIREBASE_FEEDER_CONNECTED_KEY, isConnected());
        _sourceIsButton = false;
    }

    if (isBluetoothRunning())
    {
        std::string s = isConnected() ? "true" : "false";
        _connectedCharacteristic->setValue(s);
        _connectedCharacteristic->notify();
    }

    sendConnectionChangeNotification("Feeder", isConnected());
}

std::vector<String> Feeder::getText()
{
    if (!_connected)
    {
        return {"Feeder", "Disconnected"};
    }
    else
    {
        int time;
        if (getNextTriggerTime(&time))
        {
            return {"Feeder", "Next: " + formatTime(time)};
        }
        else
        {
            return {"Feeder", "No trigger"};
        }
    }
}

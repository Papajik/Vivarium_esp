#include "feeder.h"
#include "../../../wifi/wifiProvider.h"

#include <Stepper.h>

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#define CONNECTED_KEY "feeder/c"
#define MEMORY_TRIGGER_PREFIX "feedT."

Feeder *feederPtr;
Feeder::Feeder(int position, int in_1, int in_2, int in_3, int in_4) : IModule(CONNECTED_KEY, position)
{
    printlnA("Feeder created");
    _stepper = new Stepper(FEEDER_STEPS_PER_REVOLUTION, in_1, in_3, in_2, in_4);
}

Feeder::~Feeder()
{
    delete _stepper;
}

int Feeder::asignAvailableMemoryId()
{
    for (int i = 0; i < FEEDER_MAX_TRIGERS; i++)
    {
        if (availableIds[i])
        {
            availableIds[i] = false;
            return i;
        }
    }
    return INVALID_MEMORY_ID;
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
        char time[40];
        ultoa(now, time, 10);

        firebaseService->uploadCustomData("feed/", "/" + String(time), "true");

        _feeded = false;
    }
}
void Feeder::saveSettings()
{
    _memoryProvider->saveStruct(SETTINGS_FEEDER_KEY, &_settings, sizeof(FeederSettings));
    _settingsChanged = false;
}

bool Feeder::loadSettings()
{
    if (!_memoryProvider->loadStruct(SETTINGS_FEEDER_KEY, &_settings, sizeof(FeederSettings)))
    {
        _settings = {BOX};
        saveSettings();
    }

    //TODO check if date is available rather than checking wifi connection
    if (wifiProvider->isConnected())
    {
        loadTriggersFromNVS();
        printTriggers();
    }

    return true;
}

void Feeder::removeTrigger(String key)
{
    printlnA("Trying to remove trigger " + key);
    auto it = _triggers.find(key);
    if (it != _triggers.end())
    {
        printlnA("Removing trigger");
        _memoryProvider->removeKey(String(MEMORY_TRIGGER_PREFIX) + String(it->second->storageId));
        Alarm.free(it->second->id);                 // clear alarm
        availableIds[it->second->storageId] = true; // id is available again
        _triggers.erase(_triggers.find(key));       // remove record from map
    }
    else
    {
        printlnA("trigger not found");
    }
}

void Feeder::loadTriggersFromNVS()
{
    printlnA("Load triggers from NVS");
    for (int i = 0; i < FEEDER_MAX_TRIGERS; i++)
    {
        loadTriggerFromNVS(i);
    }
}

void Feeder::loadTriggerFromNVS(int index)
{
    FeedTriggerMem enc;
    if (_memoryProvider->loadStruct(String(MEMORY_TRIGGER_PREFIX) + String(index), &enc, sizeof(FeedTriggerMem)))
    {
        printlnA("Loading feed trigger from memory");
        auto t = std::make_shared<FeedTrigger>();
        t->storageId = index;
        t->minute = enc.minute;
        t->hour = enc.hour;
        t->firebaseKey = String(enc.key);

        t->id = Alarm.alarmRepeat(t->hour, t->minute, 0, feederCallback);
        _triggers.insert({t->firebaseKey, t});
        printTrigger(t);
    }
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
        speed = 0;
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
}

void feederCallback()
{
    feederPtr->feed();
}

void Feeder::onConnectionChange()
{
    printlnA("Feeder callback");
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

void Feeder::printTrigger(std::shared_ptr<FeedTrigger> t)
{
    if (t != nullptr)
    {
        printlnA(String("ID: ") + String(t->id) + String('(') + t->firebaseKey + String(" ..- ") + String(MEMORY_TRIGGER_PREFIX) + String(t->storageId) + String(") ") +
                 String(" --> ") + String(t->hour) + String(":") + String(t->minute));
    }
    else
    {
        printlnA("Trigger nullptr");
    }
}

void Feeder::printTriggers()
{
    printlnA("All feeder triggers: ");
    for (auto &&t : _triggers)
    {
        printA(t.first);
        printA(" >>>> ");
        printTrigger(t.second);
    }
    Alarm.printAlarms();
    printlnA("");
}

void Feeder::saveTriggerToNVS(std::shared_ptr<FeedTrigger> trigger)
{
    if (trigger->storageId == INVALID_MEMORY_ID)
    {
        trigger->storageId = asignAvailableMemoryId();
        if (trigger->storageId == INVALID_MEMORY_ID)
        {
            printlnE("Maximum of feed triggers reached");
            return;
        }
    }
    FeedTriggerMem m;
    strcpy(m.key, trigger->firebaseKey.c_str());
    m.hour = trigger->hour;
    m.minute = trigger->minute;
    printA("Size of trigger = ");
    printlnA(sizeof(FeedTriggerMem));
    _memoryProvider->saveStruct(String(MEMORY_TRIGGER_PREFIX) + String(trigger->storageId), &m, sizeof(FeedTriggerMem));
}

bool Feeder::parseTime(std::shared_ptr<FeedTrigger> trigger, int time)
{
    bool triggerChanged = false;

    if (trigger->hour != time / 256)
    {
        trigger->hour = time / 256;
        triggerChanged = true;
    }
    if (trigger->minute != time % 256)
    {
        trigger->minute = time % 256;
        triggerChanged = true;
    }
    return triggerChanged;
}

std::shared_ptr<FeedTrigger> Feeder::findTrigger(String key)
{
    auto it = _triggers.find(key);
    if (it != _triggers.end())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

int Feeder::getTime(int hour, int minute)
{
    return hour * 256 + minute;
}

// Time and firebasekey
void Feeder::createTrigger(int time, String key)
{
    auto t = std::make_shared<FeedTrigger>();
    t->hour = time / 256;
    t->minute = time % 256;
    t->id = Alarm.alarmRepeat(t->hour, t->minute, 0, feederCallback);
    t->firebaseKey = key;
    _triggers.insert({key, t});
    saveTriggerToNVS(t);
}
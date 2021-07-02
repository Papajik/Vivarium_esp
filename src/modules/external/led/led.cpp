#include "led.h"

#define CONNECTED_KEY "led/c"
#define FIREBASE_COLOR_STATE "/led/color"

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#define MEMORY_TRIGGER_PREFIX "ledT."
#include "../../../wifi/wifiProvider.h"
#include <Freenove_WS2812_Lib_for_ESP32.h>

#include "../../../debug/memory.h"

#include "../../../utils/timeHelper.h"

void ledTriggerCallback()
{
    Serial.println("LED Trigger callback");
    AlarmId id = Alarm.getTriggeredAlarmId();
    if (ledModulePtr != nullptr)
    {
        uint32_t color;
        if (ledModulePtr->getTriggerColor(id, &color))
        {
            printA("Color: ");
            printlnA(String(color));
            ledModulePtr->setColor(color);
        }
    }
    else
    {
        printlnE("Led Module is nullptr");
    }
}

LedModule *ledModulePtr = nullptr;

LedModule::LedModule(int position, int pin) : IModule(CONNECTED_KEY, position)
{

    _strip = new Freenove_ESP32_WS2812(LED_COUNT, pin, LED_CHANNEL, TYPE_GRB);
    _stripConnected = _strip->begin();
}

LedModule::~LedModule()
{
    delete _strip;
    rmt_driver_uninstall((rmt_channel_t)LED_CHANNEL);
    clearAllTriggers();
}

void LedModule::clearAllTriggers()
{
    if (_triggers.empty())
        return;
    for (auto p : _triggers)
    {
        Alarm.free(p.second->id);
    }
    _triggers.clear();
}

void LedModule::setColor(uint32_t color)
{
    if (_currentColor != color)
    {
        // printlnA("new color");
        // printlnA(String(color));
        _currentColor = color;
        _settingsChanged = true;
    }
}

uint32_t LedModule::getColor()
{
    return _currentColor;
}

/// Module
void LedModule::onLoop()
{
    checkConnectionChange();

    if (isConnected())
    {
        showColor();
    }

    if (_settingsChanged)
    {
        saveSettings();
    }
}

void LedModule::showColor()
{
    if (_currentColor != _lastColor)
    {
        _lastColor = _currentColor;
        printI("LED - new color : ");
        printlnI(String(_currentColor));
        _strip->setAllLedsColor(_currentColor);

        uploadColorChange();

        if (isBluetoothRunning())
        {
            _currentColorCharacteristic->setValue(_currentColor);
            _currentColorCharacteristic->notify();
        }
        if (firebaseService != nullptr)
            firebaseService->uploadState(FIREBASE_COLOR_STATE, (int)_currentColor);
    }
}

void LedModule::uploadColorChange()
{
    printlnV("Upload color change");
    time_t now;
    time(&now);
    char time[40];
    ultoa(now, time, 10);
    if (firebaseService != nullptr)
        firebaseService->uploadCustomData("led/", "/" + String(time), String(_currentColor));
}

void LedModule::saveSettings()
{
    _memoryProvider->saveInt(SETTINGS_LED_KEY, _currentColor);
    _settingsChanged = false;
}

bool LedModule::loadSettings()
{
    _currentColor = _memoryProvider->loadInt(SETTINGS_LED_KEY, ((((uint32_t)255 << 0) | ((uint32_t)255 << 8) | ((uint32_t)255 << 16))));

    showColor();

    //TODO check if time is available instead of checking wifi connection

    if (wifiProvider->isConnected())
    {
        loadTriggersFromNVS();
        printTriggers();
    }
    return true;
}

void LedModule::printTrigger(std::shared_ptr<LedTrigger> t)
{
    if (t != nullptr)
    {
        printlnI(String("ID: ") + String(t->id) + String('(fKey: ') + t->firebaseKey + String(") --> ") + String(t->hour) + String(":") + String(t->minute) + String(" - ") +
                 String(t->color));
    }
}

void LedModule::loadTriggersFromNVS()
{
    for (int i = 0; i < LED_TRIGGERS_MAX; i++)
    {
        loadTriggerFromNVS(i);
    }
}

int LedModule::asignAvailableMemoryId()
{
    for (int i = 0; i < LED_TRIGGERS_MAX; i++)
    {
        if (availableIds[i])
        {
            availableIds[i] = false;
            return i;
        }
    }
    return INVALID_MEMORY_ID;
}

bool LedModule::getNextTriggerColor(uint32_t *color)
{

    std::shared_ptr<LedTrigger> t = getNextTrigger();
    if (t != nullptr)
    {
        *color = t->color;
        return true;
    }
    return false;
}

std::shared_ptr<LedTrigger> LedModule::getNextTrigger()
{
    tm timeinfo;
    std::shared_ptr<LedTrigger> h = nullptr;
    std::shared_ptr<LedTrigger> l = nullptr;

    if (_triggers.empty())
        return nullptr;

    if (getLocalTime(&timeinfo, 100))
    {
        int time = getTime(timeinfo.tm_hour, timeinfo.tm_min);

        for (auto p : _triggers)
        {
            int ttc = getTime(p.second->hour, p.second->minute);

            //Lookup for the lowest time
            if (ttc < time) // assign only triggers with lower time than current value
            {
                if (h == nullptr)
                {
                    if (l == nullptr)
                    {

                        l = p.second;
                    }
                    else
                    {
                        if (getTime(l->hour, l->minute) > ttc)
                        {
                            l = p.second;
                        }
                    }
                }
            }

            //Lookup for the lowest of higher times
            if (ttc > time)
            {
                if (h == nullptr)
                {
                    h = p.second;
                }
                else
                {
                    if (getTime(h->hour, h->minute) > ttc)
                    {
                        h = p.second;
                    }
                }
            }
        }
    }
    return h != nullptr ? h : l;
}

bool LedModule::getNextTriggerTime(int *time)
{
    std::shared_ptr<LedTrigger> t = getNextTrigger();
    if (t != nullptr)
    {
        *time = getTime(t->hour, t->minute);
        return true;
    }

    return false;
}

int LedModule::getTriggersCount()
{
    return _triggers.size();
}

void LedModule::saveTriggerToNVS(std::shared_ptr<LedTrigger> trigger)
{
    if (_memoryProvider == nullptr)
        return;

    if (trigger->storageId == INVALID_MEMORY_ID)
    {
        trigger->storageId = asignAvailableMemoryId();
        if (trigger->storageId == INVALID_MEMORY_ID)
        {
            printlnE("Maximum of led triggers reached");
            return;
        }
    }
    LedTriggerMem m;
    strcpy(m.key, trigger->firebaseKey.c_str());
    m.color = trigger->color;
    m.hour = trigger->hour;
    m.minute = trigger->minute;
    printA("Size of trigger = ");
    printlnA((int)sizeof(LedTriggerMem));
    _memoryProvider->saveStruct(String(MEMORY_TRIGGER_PREFIX) + String(trigger->storageId), &m, sizeof(LedTriggerMem));
}

void LedModule::loadTriggerFromNVS(int index)
{
    if (_memoryProvider == nullptr)
        return;

    LedTriggerMem enc;
    if (_memoryProvider->loadStruct(String(MEMORY_TRIGGER_PREFIX) + String(index), &enc, sizeof(LedTriggerMem)))
    {
        printlnA("Loading LED trigger from memory");
        auto t = std::make_shared<LedTrigger>();
        t->storageId = index;
        t->color = enc.color;
        t->minute = enc.minute;
        t->hour = enc.hour;
        t->firebaseKey = String(enc.key);

        t->id = Alarm.alarmRepeat(t->hour, t->minute, 0, ledTriggerCallback);
        _triggers.insert({t->firebaseKey, t});
        printTrigger(t);
    }
}

bool LedModule::getTriggerColor(uint8_t id, uint32_t *color)
{
    //  printTriggers();
    for (auto &&p : _triggers)
    {
        if (p.second->id == id)
        {
            *color = p.second->color;
            return true;
        }
    }
    return false;
}

void LedModule::printTriggers()
{

    printlnV("All led triggers: ");
    for (auto &&t : _triggers)
    {
        printV(t.first);
        printV(" >>>> ");
        printTrigger(t.second);
    }
    // Alarm.printAlarms();
    printlnV("");
}

void LedModule::removeTrigger(String key)
{
    auto it = _triggers.find(key);
    if (it != _triggers.end())
    {
        printlnI("Removing trigger");
        if (_memoryProvider != nullptr)
            _memoryProvider->removeKey(String(MEMORY_TRIGGER_PREFIX) + String(it->second->storageId));
        Alarm.free(it->second->id); // clear alarm
        if (it->second->storageId != INVALID_MEMORY_ID)
            availableIds[it->second->storageId] = true; // id is available again
        _triggers.erase(_triggers.find(key));           // remove record from map
    }
}

void LedModule::onConnectionChange()
{

    sendConnectionChangeNotification("LED", isConnected());
    if (isConnected())
    {
        // Reinit color on new connection + upload data to RTDB
        showColor();
        uploadColorChange();
    }

    if (_sourceIsButton)
    {
        if (firebaseService != nullptr)
            firebaseService->uploadState(FIREBASE_LED_CONNECTED_KEY, isConnected());
        _sourceIsButton = false;
    }

    if (isBluetoothRunning())
    {
        std::string s = isConnected() ? "true" : "false";
        _connectedCharacteristic->setValue(s);
        _connectedCharacteristic->notify();
    }
}

void LedModule::createTrigger(int time, int color, String key)
{
    std::shared_ptr<LedTrigger> t = std::make_shared<LedTrigger>();
    t->color = color;
    t->hour = time / 256;
    t->minute = time % 256;
    t->firebaseKey = key;
    t->id = Alarm.alarmRepeat(t->hour, t->minute, 0, ledTriggerCallback);
    _triggers.insert({key, t});
    saveTriggerToNVS(t);
}

std::shared_ptr<LedTrigger> LedModule::findTrigger(String key)
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

bool LedModule::isStripConnected()
{
    return _stripConnected;
}

std::vector<String> LedModule::getText()
{
    if (!_connected)
    {
        return {"LED", "Disconnected"};
    }
    else
    {
        int time;
        if (getNextTriggerTime(&time))
        {
            return {"LED", "Next: " + formatTime(time)};
        }
        else
        {
            return {"LED", "No trigger"};
        }
    }
}
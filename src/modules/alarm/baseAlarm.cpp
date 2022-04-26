/**
* @file baseAlarm.cpp
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-10-24
* 
* @copyright Copyright (c) 2021
* 
*/

#include "baseAlarm.h"
#include "../../utils/timeHelper.h"
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include "../../memory/memory_provider_internal.h"
#include <Firebase_ESP_Client.h>

template <typename T>
BaseAlarm<T>::BaseAlarm(TriggerCallback callback, MemoryProvider *provider, std::string prefix)
    : _provider(provider), _callback(callback), _memoryPrefix(prefix)
{
    BaseAlarm<T>::triggersMutex = xSemaphoreCreateMutex();
    xSemaphoreGive(BaseAlarm<T>::triggersMutex);
}

template <typename T>
BaseAlarm<T>::~BaseAlarm()
{
    clearTriggers();
}

template <typename T>
bool BaseAlarm<T>::getNextTriggerTime(int *time)
{

    std::shared_ptr<T> t = getNextTrigger();
    if (t != nullptr)
    {
        *time = getTime(t->hour, t->minute);
        return true;
    }

    return false;
}

template <typename T>
std::shared_ptr<T> BaseAlarm<T>::getNextTrigger()
{
    if (_triggers.empty())
        return nullptr;

    tm timeinfo;
    std::shared_ptr<T> h = nullptr;
    std::shared_ptr<T> l = nullptr;

    if (getLocalTime(&timeinfo, 100))
    {
        int time = getTime(timeinfo.tm_hour, timeinfo.tm_min);
        lockSemaphore("getNextTrigger");
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
        unlockSemaphore();
    }
    return h != nullptr ? h : l;
}

template <typename T>
void BaseAlarm<T>::clearTriggers()
{
    if (_triggers.empty())
        return;
    lockSemaphore("clear led");
    for (auto p : _triggers)
    {
        Alarm.free(p.second->id);
    }
    _triggers.clear();
    unlockSemaphore();
}

template <typename T>
void BaseAlarm<T>::removeTrigger(String firebaseKey)
{
    lockSemaphore("removeTrigger");
    auto it = _triggers.find(firebaseKey);

    if (it != _triggers.end())
    {
        printlnD("Removing trigger");
        if (_provider != nullptr)
            _provider->removeKey(String(MAX_TRIGGERS) + String(it->second->storageId));
        Alarm.free(it->second->id); // clear alarm
        if (it->second->storageId != INVALID_MEMORY_ID)
            availableIds[it->second->storageId] = true; // id is available again
        _triggers.erase(it);                            // remove record from map
    }
    unlockSemaphore();
}

template <typename T>
int BaseAlarm<T>::getTriggersCount() { return _triggers.size(); }

template <typename T>
void BaseAlarm<T>::lockSemaphore(std::string owner)
{
    debugV("Trying to lock semaphore by %s", owner.c_str());
    xSemaphoreTake(BaseAlarm<T>::triggersMutex, portMAX_DELAY);
    debugV("Lock semaphore by %s", owner.c_str());
    _owner = owner;
}

template <typename T>
void BaseAlarm<T>::unlockSemaphore()
{
    xSemaphoreGive(BaseAlarm<T>::triggersMutex);
    debugV("Unlock semaphore from %s", _owner.c_str());
}

template <typename T>
TriggerCallback BaseAlarm<T>::getCallback()
{
    return _callback;
}

template <typename T>
int BaseAlarm<T>::getAvailableMemoryId()
{
    for (int i = 0; i < MAX_TRIGGERS; i++)
    {
        if (availableIds[i])
        {
            availableIds[i] = false;
            return i;
        }
    }
    return INVALID_MEMORY_ID;
}

template <typename T>
void BaseAlarm<T>::loadTriggersFromNVS()
{
    printlnA("Load triggers from NVS");
    for (int i = 0; i < MAX_TRIGGERS; i++)
    {
        loadTriggerFromNVS(i);
    }
}

template <typename T>
bool BaseAlarm<T>::updateTriggerTime(std::shared_ptr<T> trigger, int time)
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

template <typename T>
std::shared_ptr<T> BaseAlarm<T>::findTrigger(String key)
{
    lockSemaphore("findTrigger");
    auto it = _triggers.find(key);
    unlockSemaphore();
    if (it != _triggers.end())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

template <typename T>
void BaseAlarm<T>::parseTriggersJson(FirebaseJson *json)
{
    printlnV("Iterating through json");
    std::vector<String> toDelete;
    ;
    // 1. check all existing triggers;
    lockSemaphore("parseTriggersJson1");
    for (auto &&it : _triggers)
    {
        printlnA("Checking key = " + it.first);
        FirebaseJsonData triggerData;

        // If triggers with key exists in json
        if (json->get(triggerData, "/" + it.first, true))
        {

            FirebaseJson innerJson;
            triggerData.getJSON(innerJson);
            bool triggerChanged = updateTriggerFromJson(it.second, &innerJson);

            if (triggerChanged)
            {
                printlnA("Trigger " + it.first + " changed, reseting timer");
                Alarm.free(it.second->id);
                it.second->id = Alarm.alarmRepeat(it.second->hour, it.second->minute, 0, getCallback());
                saveTriggerToNVS(it.second);
            }
        }
        else
        {
            printlnA("Deleting trigger " + it.first);
            toDelete.push_back(it.first);
        }
    }
    unlockSemaphore();

    // Delete all marked triggers
    for (String s : toDelete)
    {
        removeTrigger(s);
    }

    //check for new triggers
    createNewTriggersFromJson(json);
    printTriggers();
}

template <typename T>
void BaseAlarm<T>::parseTriggerJson(FirebaseJson *json, String path)
{
    FirebaseJsonData jsonData;
    printlnI("LED  parseTriggerJson");
    int index = path.lastIndexOf("/");
    String triggerKey = path.substring(index + 1);

    printlnV("Parse 1 trigger json callback");
    printlnV("TriggerKey = " + triggerKey);

    // adding new trigger;
    lockSemaphore("parseTriggerJson1");
    auto it = _triggers.find(triggerKey);
    unlockSemaphore();
    if (it == _triggers.end())
    {
        createTriggerFromJson(json, triggerKey);
    }
}

template <typename T>
void BaseAlarm<T>::parseTriggerValue(String key, String value)
{
    debugD("Parsing value, key = %s, value = %s\n", key.c_str(), value.c_str());

    int index = key.lastIndexOf("/");

    String jsonKey = key.substring(index + 1);
    String triggerKey = key.substring(0, index);
    triggerKey = triggerKey.substring(triggerKey.lastIndexOf("/") + 1);

    printlnD("jsonKey = " + jsonKey);
    printlnD("triggerKey = " + triggerKey);
    printlnD("value = " + value);

    if (value == "null")
    {
        printlnD("Removing key");
        removeTrigger(jsonKey);
    }
    else
    {
        lockSemaphore("parseValue");
        auto it = _triggers.find(triggerKey);
        unlockSemaphore();
        if (it != _triggers.end())
        {
            Alarm.free(it->second->id);
            parseTriggerCustomValue(it->second, jsonKey, value);
            it->second->id = Alarm.alarmRepeat(it->second->hour, it->second->minute, 0, getCallback());

            saveTriggerToNVS(it->second);
        }
    }
    printTriggers();
}

template <typename T>
void BaseAlarm<T>::printTriggers()
{
    for (auto &&t : _triggers)
    {
        printI(t.first);
        printI(" >>>> ");
        printTrigger(t.second);
    }
    printlnI("");
}
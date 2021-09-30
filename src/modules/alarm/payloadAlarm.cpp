#include "payloadAlarm.h"
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <Firebase_ESP_Client.h>
#include "../../memory/memory_provider_internal.h"

#define JSON_STEP 3

template <typename K>
PayloadAlarm<K>::PayloadAlarm(TriggerCallback callback, MemoryProvider *provider, std::string prefix)
    : BaseAlarm<PayloadTrigger<K>>(callback, provider, prefix) {}

template <typename K>
bool PayloadAlarm<K>::getTriggerPayload(uint8_t id, K *payload)
{
    BaseAlarm<PayloadTrigger<K>>::lockSemaphore("getTriggerPayload");
    for (auto &&p : BaseAlarm<PayloadTrigger<K>>::_triggers)
    {
        if (p.second->id == id)
        {
            *payload = p.second->payload;
            BaseAlarm<PayloadTrigger<K>>::unlockSemaphore();
            return true;
        }
    }
    BaseAlarm<PayloadTrigger<K>>::unlockSemaphore();
    return false;
}

template <typename T>
bool PayloadAlarm<T>::getNextTriggerPayload(T *payload)
{

    std::shared_ptr<PayloadTrigger<T>> t = BaseAlarm<PayloadTrigger<T>>::getNextTrigger();
    if (t != nullptr)
    {
        *payload = t->payload;
        return true;
    }
    return false;
}

template <typename T>
void PayloadAlarm<T>::createTrigger(int time, String key, T payload)
{
    printlnA("Creating trigger" + key);

    std::shared_ptr<PayloadTrigger<T>> t = std::make_shared<PayloadTrigger<T>>();
    t->payload = payload;
    t->hour = time / 256;
    t->minute = time % 256;
    t->firebaseKey = key;
    t->id = Alarm.alarmRepeat(t->hour, t->minute, 0, BaseAlarm<PayloadTrigger<T>>::getCallback());
    BaseAlarm<PayloadTrigger<T>>::lockSemaphore("createTrigger");
    BaseAlarm<PayloadTrigger<T>>::_triggers.insert({key, t});
    BaseAlarm<PayloadTrigger<T>>::unlockSemaphore();
    saveTriggerToNVS(t);
}

template <typename T>
struct PayloadTriggerMemory
{
    int hour;
    int minute;
    T payload;
    char key[25];
};

template <typename T>
void PayloadAlarm<T>::saveTriggerToNVS(std::shared_ptr<PayloadTrigger<T>> trigger)
{
    printlnA("saveTriggerToNVS");
    if (BaseAlarm<PayloadTrigger<T>>::_provider == nullptr)
        return;

    if (trigger->storageId == INVALID_MEMORY_ID)
    {
        trigger->storageId = BaseAlarm<PayloadTrigger<T>>::getAvailableMemoryId();
        if (trigger->storageId == INVALID_MEMORY_ID)
        {
            printlnE("Maximum of led triggers reached");
            return;
        }
    }
    PayloadTriggerMemory<T> m;
    strcpy(m.key, trigger->firebaseKey.c_str());
    m.payload = trigger->payload;
    m.hour = trigger->hour;
    m.minute = trigger->minute;
    debugA("Size of trigger =  %d", (int)sizeof(PayloadTriggerMemory<T>));
    BaseAlarm<PayloadTrigger<T>>::_provider->saveStruct(String(BaseAlarm<PayloadTrigger<T>>::_memoryPrefix.c_str()) + String(trigger->storageId), &m, sizeof(PayloadTriggerMemory<T>));
}

template <typename T>
void PayloadAlarm<T>::loadTriggerFromNVS(int index)
{
    if (BaseAlarm<PayloadTrigger<T>>::_provider == nullptr)
        return;

    PayloadTriggerMemory<T> enc;
    if (BaseAlarm<PayloadTrigger<T>>::_provider->loadStruct(String(BaseAlarm<PayloadTrigger<T>>::_memoryPrefix.c_str()) + String(index), &enc, sizeof(PayloadTriggerMemory<T>)))
    {
        auto t = std::make_shared<PayloadTrigger<T>>();
        t->storageId = index;
        t->minute = enc.minute;
        t->payload = enc.payload;
        t->hour = enc.hour;
        t->firebaseKey = String(enc.key);

        t->id = Alarm.alarmRepeat(t->hour, t->minute, 0, BaseAlarm<PayloadTrigger<T>>::getCallback());
        BaseAlarm<PayloadTrigger<T>>::lockSemaphore("loadTriggerFromNVS");
        BaseAlarm<PayloadTrigger<T>>::_triggers.insert({t->firebaseKey, t});
        BaseAlarm<PayloadTrigger<T>>::unlockSemaphore();
        printTrigger(t);
    }
}

template <typename T>
void PayloadAlarm<T>::createNewTriggersFromJson(FirebaseJson *json)
{
    int type;
    String key;
    String value;
    FirebaseJsonData innerData;
    FirebaseJson innerJson;

    int len = json->iteratorBegin();
    if (len % JSON_STEP == 0)
    {
        printlnV("JSON Valid");
        for (int i = 0; i < len; i += JSON_STEP)
        {
            json->iteratorGet(i, type, key, value);
            BaseAlarm<PayloadTrigger<T>>::lockSemaphore("createNewTriggersFromJson");
            auto it = BaseAlarm<PayloadTrigger<T>>::_triggers.find(key);
            BaseAlarm<PayloadTrigger<T>>::unlockSemaphore();
            if (it == BaseAlarm<PayloadTrigger<T>>::_triggers.end()) // Check if map contain this key
            {
                if (type == FirebaseJson::JSON_OBJECT)
                {

                    auto trigger = std::make_shared<PayloadTrigger<T>>();
                    innerJson.setJsonData(value);

                    int time;

                    if (innerJson.get(innerData, "/time", false))
                    {
                        time = innerData.intValue;
                    }
                    else
                    {
                        printlnW("No time in json");
                        continue;
                    }
                    T payload;
                    if (getPayloadFromJson(&innerJson, payload))
                    {
                        createTrigger(time, key, payload);
                    }
                }
            }
        }
    }
}

template <typename T>
void PayloadAlarm<T>::printTrigger(std::shared_ptr<PayloadTrigger<T>> t)
{
    if (t != nullptr)
    {
        printA("ID: ");
        printA(t->id);
        printA("(fKey: ");
        printA(t->firebaseKey);
        printA(") --> ");
        printA(t->hour);
        printA(":");
        printA(t->minute);
        printA(" - ");
        printlnA(t->payload);
    }
}

template <typename T>
void PayloadAlarm<T>::createTriggerFromJson(FirebaseJson *json, String triggerKey)
{
    printlnA("Adding new trigger");
    int time;
    T payload;
    FirebaseJsonData jsonData;
    if (json->get(jsonData, "/time", false))
    {
        time = jsonData.intValue;
    }
    else
    {
        printlnW("NO time in json");
        return;
    }

    if (!getPayloadFromJson(json, payload))
    {
        printlnA("No goal in json");
        return;
    }

    createTrigger(time, triggerKey, payload);
}

template <typename T>
bool PayloadAlarm<T>::updateTriggerFromJson(std::shared_ptr<PayloadTrigger<T>> trigger, FirebaseJson *json)
{
    T payload;

    if (!getPayloadFromJson(json, payload))
    {
        // if json doesn't contain payload then this json is corrupted.
        return false;
    }

    FirebaseJsonData data;
    bool changed = payload != trigger->payload;

    if (json->get(data, "/time"))
    {
        if (trigger->hour != data.intValue / 256)
        {
            trigger->hour = data.intValue / 256;
            changed = true;
        }
        if (trigger->minute != data.intValue % 256)
        {
            trigger->minute = data.intValue % 256;
            changed = true;
        }
    }
    else
    {
        return false;
    }

    trigger->payload = payload;
    return changed;
}

template <typename T>
void PayloadAlarm<T>::parseTriggerCustomValue(std::shared_ptr<PayloadTrigger<T>> trigger, String key, String value)
{
    printlnA("Editing " + key);
    T payload;
    if (getPayloadFromValue(key, value, payload))
    {
        printlnA("Got payload:" + String(payload));
        trigger->payload = payload;
    }

    if (key == "time")
    {
        
        trigger->hour = (int)value.toInt() / 256;
        trigger->minute = (int)value.toInt() % 256;
    }
}
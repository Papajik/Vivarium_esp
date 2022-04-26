#include "state.h"
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <Time.h>
#include <limits>

#include <freertos/task.h>

StateStorage stateStorage;

StateStorage::StateStorage()
{
}

StateStorage::~StateStorage()
{
    uint32t_items.clear();
}

int StateStorage::setValue(String key, uint32_t value)
{
    return setValueWithCallback(key, value, NULL);
}

int StateStorage::setValue(String key, float value)
{
    // printlnA("setting float value. key = " + key + ", value = " + String(value));
    return setValueWithCallback(key, value, NULL);
}

int StateStorage::setValue(String key, bool value)
{
    return setValueWithCallback(key, value, NULL);
}

int StateStorage::setCallback(String key, ChangeCallback *callback, item_type type)
{
    switch (type)
    {

    case type_bool:
    {
        auto it = bool_items.find(key);
        if (it == bool_items.end())
        {
            state_item<bool> item;
            item.changeCallback = callback;
            bool_items.insert({key, item});
            return 0;
        }
        else
        {
            it->second.changeCallback = callback;
            return 1;
        }
    }

    case type_float:
    {
        auto it = float_items.find(key);
        if (it == float_items.end())
        {
            state_item<float> item;
            item.changeCallback = callback;
            float_items.insert({key, item});
            return 0;
        }
        else
        {
            it->second.changeCallback = callback;
            return 1;
        }
    }
    case type_uint32:
    {
        auto it = uint32t_items.find(key);
        if (it == uint32t_items.end())
        {
            state_item<uint32_t> item;
            item.changeCallback = callback;
            uint32t_items.insert({key, item});
            return 0;
        }
        else
        {
            it->second.changeCallback = callback;
            return 1;
        }
    }
    default:
    {
        return -1;
    }
    }
}

int StateStorage::setValueWithCallback(String key, uint32_t value, ChangeCallback *callback)
{
    updateTime();

    auto it = uint32t_items.find(key);

    if (it == uint32t_items.end())
    {
        // Add new item to storage

        state_item<uint32_t> item;
        item.value = value;
        item.changeCallback = callback;

        uint32t_items.insert({key, item});
        return 0;
    }
    else
    {
        // Edit already stored item
        it->second.value = value;
        if (callback != NULL)
        {
            it->second.changeCallback = callback;
        }
        //call callback if set (from this call or any previous)
        if (it->second.changeCallback != NULL)
        {
            item_value tmp_value;
            tmp_value.i = value;
            it->second.changeCallback->callback(tmp_value);
        }

        return 1;
    }
}

int StateStorage::setValueWithCallback(String key, float value, ChangeCallback *callback)
{
    updateTime();
    auto it = float_items.find(key);

    if (it == float_items.end())
    {
        // Add new item to storage

        state_item<float> item;
        item.value = value;
        item.changeCallback = callback;

        float_items.insert({key, item});
        return 0;
    }
    else
    {
        // Edit already stored item
        it->second.value = value;
        if (callback != NULL)
        {
            it->second.changeCallback = callback;
        }
        //call callback if set (from this call or any previous)
        if (it->second.changeCallback != NULL)
        {

            item_value tmp_value;
            tmp_value.f = value;
            it->second.changeCallback->callback(tmp_value);
        }

        return 1;
    }
}

int StateStorage::setValueWithCallback(String key, bool value, ChangeCallback *callback)
{
    updateTime();
    auto it = bool_items.find(key);

    if (it == bool_items.end())
    {
        // Add new item to storage

        state_item<bool> item;
        item.value = value;
        item.changeCallback = callback;

        bool_items.insert({key, item});
        return 0;
    }
    else
    {
        // Edit already stored item
        it->second.value = value;
        if (callback != NULL)
        {
            it->second.changeCallback = callback;
        }

        //call callback if set (from this call or any previous)
        if (it->second.changeCallback != NULL)
        {
            item_value tmp_value;
            tmp_value.b = value;
            it->second.changeCallback->callback(tmp_value);
        }

        return 1;
    }
}

int StateStorage::removeItem(String key, item_type type)
{
    switch (type)
    {
    case type_bool:
    {
        auto it = bool_items.find(key);
        if (it == bool_items.end())
        {
            return 0;
        }
        else
        {
            bool_items.erase(it);
            return 1;
        }
    }
    case type_float:
    {
        auto it = float_items.find(key);
        if (it == float_items.end())
        {
            return 0;
        }
        else
        {
            float_items.erase(it);
            return 1;
        }
    }
    case type_uint32:
    {
        auto it = uint32t_items.find(key);
        if (it == uint32t_items.end())
        {
            return 0;
        }
        else
        {
            uint32t_items.erase(it);
            return 1;
        }
    }
    default:
        return -1;
    }
}

void StateStorage::updateTime()
{
    time(&last_update);
    if (updateTimeCallback != NULL)
    {
        updateTimeCallback(last_update);
    }
}

void StateStorage::setUpdateTimeCallback(UpdateTimeCallback callback)
{
    updateTimeCallback = callback;
}

bool StateStorage::getValue(String key, uint32_t *value)
{
    auto it = uint32t_items.find(key);
    if (it == uint32t_items.end())
    {
        *value = std::numeric_limits<uint32_t>::min();
        return false;
    }
    else
    {
        *value = it->second.value;
        return true;
    }
}

bool StateStorage::getValue(String key, float *value)
{
    auto it = float_items.find(key);
    if (it == float_items.end())
    {
        *value = std::numeric_limits<float>::min();
        return false;
    }
    else
    {
        *value = it->second.value;
        return true;
    }
}

bool StateStorage::getValue(String key, bool *value)
{
    auto it = bool_items.find(key);
    if (it == bool_items.end())
    {
        *value = false;
        return false;
    }
    else
    {
        *value = it->second.value;
        return true;
    }
}

time_t StateStorage::getLastUpdate()
{
    return last_update;
}

void StateStorage::printState()
{
}

void StateStorage::clear()
{
    bool_items.clear();
    float_items.clear();
    uint32t_items.clear();
}
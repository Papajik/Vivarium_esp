#include "state.h"
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <Time.h>
#include <limits>

#include <freertos/task.h>

StateStorage stateStorage;
portMUX_TYPE uintMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE floatMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE boolMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timeMux = portMUX_INITIALIZER_UNLOCKED;

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
        portENTER_CRITICAL(&boolMux);
        auto it = bool_items.find(key);
        if (it == bool_items.end())
        {
            state_item<bool> item;
            item.changeCallback = callback;
            bool_items.insert({key, item});
            portEXIT_CRITICAL(&boolMux);
            return 0;
        }
        else
        {
            it->second.changeCallback = callback;
            portEXIT_CRITICAL(&boolMux);
            return 1;
        }
    }

    case type_float:
    {
        portENTER_CRITICAL(&floatMux);
        auto it = float_items.find(key);
        if (it == float_items.end())
        {
            state_item<float> item;
            item.changeCallback = callback;
            float_items.insert({key, item});
            portEXIT_CRITICAL(&floatMux);
            return 0;
        }
        else
        {
            it->second.changeCallback = callback;
            portEXIT_CRITICAL(&floatMux);
            return 1;
        }
    }
    case type_uint32:
    {
        portENTER_CRITICAL(&uintMux);
        auto it = uint32t_items.find(key);
        if (it == uint32t_items.end())
        {
            state_item<uint32_t> item;
            item.changeCallback = callback;
            uint32t_items.insert({key, item});
            portEXIT_CRITICAL(&uintMux);
            return 0;
        }
        else
        {
            it->second.changeCallback = callback;
            portEXIT_CRITICAL(&uintMux);
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

    portENTER_CRITICAL(&uintMux);
    auto it = uint32t_items.find(key);

    if (it == uint32t_items.end())
    {
        // Add new item to storage

        state_item<uint32_t> item;
        item.value = value;
        item.changeCallback = callback;

        uint32t_items.insert({key, item});
        portEXIT_CRITICAL(&uintMux);
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
        portEXIT_CRITICAL(&uintMux);
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
    portENTER_CRITICAL(&floatMux);
    auto it = float_items.find(key);

    if (it == float_items.end())
    {
        // Add new item to storage

        state_item<float> item;
        item.value = value;
        item.changeCallback = callback;

        float_items.insert({key, item});
        portEXIT_CRITICAL(&floatMux);
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
        portEXIT_CRITICAL(&floatMux);
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
    portENTER_CRITICAL(&boolMux);
    auto it = bool_items.find(key);

    if (it == bool_items.end())
    {
        // Add new item to storage

        state_item<bool> item;
        item.value = value;
        item.changeCallback = callback;

        bool_items.insert({key, item});
        portEXIT_CRITICAL(&boolMux);
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
        portEXIT_CRITICAL(&boolMux);

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

int StateStorage::removeUint32t(String key, item_type type)
{
    switch (type)
    {
    case type_bool:
    {
        portENTER_CRITICAL(&boolMux);
        auto it = bool_items.find(key);
        if (it == bool_items.end())
        {
            portEXIT_CRITICAL(&boolMux);
            return 0;
        }
        else
        {
            bool_items.erase(it);

            portEXIT_CRITICAL(&boolMux);
            return 1;
        }
    }
    case type_float:
    {
        portENTER_CRITICAL(&floatMux);
        auto it = float_items.find(key);
        if (it == float_items.end())
        {
            portEXIT_CRITICAL(&floatMux);
            return 0;
        }
        else
        {
            float_items.erase(it);

            portEXIT_CRITICAL(&floatMux);
            return 1;
        }
    }
    case type_uint32:
    {
        portENTER_CRITICAL(&uintMux);
        auto it = uint32t_items.find(key);
        if (it == uint32t_items.end())
        {
            portEXIT_CRITICAL(&uintMux);
            return 0;
        }
        else
        {
            uint32t_items.erase(it);

            portEXIT_CRITICAL(&uintMux);
            return 1;
        }
    }
    default:
        return -1;
    }
}

void StateStorage::updateTime()
{
    portENTER_CRITICAL(&timeMux);
    time(&last_update);
    portEXIT_CRITICAL(&timeMux);
    if (updateTimeCallback != NULL)
    {
        updateTimeCallback(last_update);
    }
}

void StateStorage::setUpdateTimeCallback(UpdateTimeCallback callback)
{
    portENTER_CRITICAL(&timeMux);
    updateTimeCallback = callback;
    portEXIT_CRITICAL(&timeMux);
}

bool StateStorage::getValue(String key, uint32_t *value)
{
    portENTER_CRITICAL(&uintMux);
    auto it = uint32t_items.find(key);
    if (it == uint32t_items.end())
    {
        *value = std::numeric_limits<uint32_t>::min();
        portEXIT_CRITICAL(&uintMux);
        return false;
    }
    else
    {
        *value = it->second.value;
        ;
        portEXIT_CRITICAL(&uintMux);
        return true;
    }
}

bool StateStorage::getValue(String key, float *value)
{
    portENTER_CRITICAL(&floatMux);
    auto it = float_items.find(key);
    if (it == float_items.end())
    {
        *value = std::numeric_limits<float>::min();
        portEXIT_CRITICAL(&floatMux);
        return false;
    }
    else
    {
        *value = it->second.value;
        portEXIT_CRITICAL(&floatMux);
        return true;
    }
}

bool StateStorage::getValue(String key, bool *value)
{
    portENTER_CRITICAL(&boolMux);
    auto it = bool_items.find(key);
    if (it == bool_items.end())
    {
        *value = false;

        portEXIT_CRITICAL(&boolMux);
        return false;
    }
    else
    {
        *value = it->second.value;

        portEXIT_CRITICAL(&boolMux);
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

// void StateStorage::getStateJson(JsonVariant variant)
// {
//     printlnD("Getting state json");
//     getStateJsonFromMap(&float_items, variant);
//     getStateJsonFromMap(&bool_items, variant);
//     getStateJsonFromMap(&uint32t_items, variant);
// }

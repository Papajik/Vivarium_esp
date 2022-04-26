
#include "memory_provider_internal.h"
#include <Preferences.h>

#include <nvs.h>

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

MemoryProviderInternal::MemoryProviderInternal() : _preferences(new Preferences())
{
    preferencesMutex = xSemaphoreCreateMutex();
    xSemaphoreGive(preferencesMutex);
}

MemoryProviderInternal::~MemoryProviderInternal()
{
    delete _preferences;
}

void MemoryProviderInternal::begin()
{
    _preferences->begin(name.c_str(), false);
}

void MemoryProviderInternal::init(String n = "vivarium")
{
    name = n;

    nvs_stats_t nvs_stats;
    nvs_get_stats(NULL, &nvs_stats);
    printA("Count: UsedEntries = ");
    printlnA(nvs_stats.used_entries);
    printA("FreeEntries = ");
    printlnA(nvs_stats.free_entries);
    printA("AllEntries = ");
    printlnA(nvs_stats.total_entries);

    lockSemaphore("init");
    if (_preferences->isKey(NUMBER_OF_WRITES_KEY))
    {
        _writeCount = _preferences->getUInt(NUMBER_OF_WRITES_KEY, 0);
    }
    else
    {
        _writeCount = 0;
    }

    if (_preferences->isKey(NUMBER_OF_BYTES_KEY))
    {
        _bytesWritten = _preferences->getUInt(NUMBER_OF_BYTES_KEY, 0);
    }
    else
    {
        _bytesWritten = 0;
    }
    unlockSemaphore();
}

void MemoryProviderInternal::end()
{
    _preferences->end();
}

bool MemoryProviderInternal::loadStruct(String key, void *value, size_t len)
{
    // begin();
    lockSemaphore("loadStruct_" + key);
    if (_preferences->isKey(key.c_str()))
    {
        size_t structlength = _preferences->getBytesLength(key.c_str());
        if (structlength != len)
        {
            printlnA("Error");
            printlnE("Struct has incorrect size");
            unlockSemaphore();
            return false;
        }
        else
        {
            _preferences->getBytes(key.c_str(), value, len);
            // debugA("Loaded struct under '%s' key", key.c_str());
            unlockSemaphore();
            return true;
        }
    }
    else
    {
        // debugA("No struct saved under '%s' key", key.c_str());
        unlockSemaphore();
        return false;
    }
}

void MemoryProviderInternal::saveStruct(String key, const void *value, size_t len)
{

    printA("Saving struct under key ");
    printlnA(key);
    lockSemaphore("saveStruct_" + key);
    size_t bytes = _preferences->putBytes(key.c_str(), value, len);
    unlockSemaphore();
    debugA("Saved %d bytes", bytes);
    _incrementWrites();
    _incrementBytes(bytes);
}

void MemoryProviderInternal::_incrementWrites()
{
    _writeCount++;
    printD("New number of writes = ");
    printD(_writeCount);
    printD(" (");
    lockSemaphore("incrementWrites");
    printD(_preferences->putUInt(NUMBER_OF_WRITES_KEY, _writeCount));
    unlockSemaphore();
    printlnD(" bytes)");
}

void MemoryProviderInternal::_incrementBytes(int bytes)
{
    _bytesWritten += bytes;
    printD("New number of bytes = ");
    printlnD(_bytesWritten);
    lockSemaphore("incrementBytes");
    _preferences->putUInt(NUMBER_OF_BYTES_KEY, _bytesWritten);
    unlockSemaphore();
}

void MemoryProviderInternal::saveString(String key, String value)
{
    printD("MP: Saving string ");
    printD(value);
    lockSemaphore("saveString_" + key);
    if (!_preferences->isKey(key.c_str()))
    {
        size_t bytes = _preferences->putString(key.c_str(), value);
        unlockSemaphore();
        debugD("(%d bytes) under key %s\n", bytes, key.c_str());
        _incrementWrites();
        _incrementBytes(bytes);
    }
    else if (_preferences->getString(key.c_str(), String("")) != value)
    {
        size_t bytes = _preferences->putString(key.c_str(), value);
        debugD("(%d bytes) under key %s\n", bytes, key.c_str());
        unlockSemaphore();
        _incrementWrites();
        _incrementBytes(bytes);
    }
    else
    {
        unlockSemaphore();
        printlnD(" - string is already stored");
    }
}

String MemoryProviderInternal::loadString(String key, String defaultValue)
{
    lockSemaphore("loadString_" + key);

    if (_preferences->isKey(key.c_str()))
    {
        printlnA("NVS - key " + key + " exists");
        String s = _preferences->getString(key.c_str(), defaultValue);
        unlockSemaphore();
        return s;
    }
    else
    {
        printlnA("NVS - key " + key + " doesn't exist");
        unlockSemaphore();
        return defaultValue;
    }
}

void MemoryProviderInternal::saveBool(String key, bool value)
{
    printA("MP: Saving bool ");
    printA(value ? "true" : "false");
    lockSemaphore("saveBool_" + key);
    if (!_preferences->isKey(key.c_str()))
    {
        size_t bytes = _preferences->putBool(key.c_str(), value);
        unlockSemaphore();
        debugD("(%d bytes) under key %s\n", bytes, key.c_str());
        _incrementWrites();
        _incrementBytes(bytes);
    }
    else if (_preferences->getBool(key.c_str()) != value)
    {
        size_t bytes = _preferences->putBool(key.c_str(), value);
        unlockSemaphore();
        debugD("(%d bytes) under key %s\n", bytes, key.c_str());
        _incrementWrites();
        _incrementBytes(bytes);
    }
    else
    {
        unlockSemaphore();
    }
}
bool MemoryProviderInternal::loadBool(String key, bool defaultValue)
{
    lockSemaphore("loadBool_" + key);
    if (_preferences->isKey(key.c_str()))
    {
        bool s = _preferences->getBool(key.c_str(), defaultValue);
        unlockSemaphore();
        return s;
    }
    else
    {
        unlockSemaphore();
        return defaultValue;
    }
}

void MemoryProviderInternal::saveInt(String key, uint32_t value)
{
    debugD("MP: Saving int: %d\n", value);
    lockSemaphore("saveInt_" + key);
    if (!_preferences->isKey(key.c_str()))
    {
        size_t bytes = _preferences->putUInt(key.c_str(), value);
        unlockSemaphore();
        debugD("(%d bytes) under key %s\n", bytes, key.c_str());
        _incrementWrites();
        _incrementBytes(bytes);
    }
    else if (_preferences->getUInt(key.c_str(), value) != value)
    {
        size_t bytes = _preferences->putUInt(key.c_str(), value);
        unlockSemaphore();
        debugD("(%d bytes) under key %s\n", bytes, key.c_str());
        _incrementWrites();
        _incrementBytes(bytes);
    }
    else
    {
        unlockSemaphore();
        printlnD(" - int already stored");
    }
}
uint32_t MemoryProviderInternal::loadInt(String key, uint32_t defaultValue)
{
    lockSemaphore("loadInt_" + key);
    if (_preferences->isKey(key.c_str()))
    {
        uint32_t t = _preferences->getUInt(key.c_str(), defaultValue);
        unlockSemaphore();
        return t;
    }
    else
    {
        unlockSemaphore();
        return defaultValue;
    }
}

void MemoryProviderInternal::saveFloat(String key, float value)
{
    printD("MP: Saving float ");
    printD(value);
    lockSemaphore("saveFloat_" + key);
    if (!_preferences->isKey(key.c_str()))
    {
        size_t bytes = _preferences->putFloat(key.c_str(), value);
        unlockSemaphore();
        debugD("(%d bytes) under key %s\n", bytes, key.c_str());
        _incrementWrites();
        _incrementBytes(bytes);
    }
    else if (_preferences->getFloat(key.c_str(), value) != value)
    {
        size_t bytes = _preferences->putFloat(key.c_str(), value);
        unlockSemaphore();
        debugD("(%d bytes) under key %s\n", bytes, key.c_str());
        _incrementWrites();
        _incrementBytes(bytes);
    }
    else
    {
        unlockSemaphore();
        printlnD(" - float already stored");
    }
}

float MemoryProviderInternal::loadFloat(String key, float defaultValue)
{
    lockSemaphore("loadFloat_" + key);
    if (_preferences->isKey(key.c_str()))
    {
        float f = _preferences->getFloat(key.c_str(), defaultValue);
        unlockSemaphore();
        return f;
    }
    else
    {
        unlockSemaphore();
        return defaultValue;
    }
}

void MemoryProviderInternal::saveDouble(String key, double value)
{
    printD("MP: Saving double ");
    printlnD(value);
    lockSemaphore("saveDouble_" + key);
    if (!_preferences->isKey(key.c_str()))
    {
        size_t bytes = _preferences->putDouble(key.c_str(), value);
        unlockSemaphore();
        debugD("(%d bytes) under key %s\n", bytes, key.c_str());
        _incrementWrites();
        _incrementBytes(bytes);
    }
    else if (_preferences->getDouble(key.c_str(), value) != value)
    {
        size_t bytes = _preferences->putDouble(key.c_str(), value);
        unlockSemaphore();
        debugD("(%d bytes) under key %s\n", bytes, key.c_str());
        _incrementWrites();
        _incrementBytes(bytes);
    }
    else
    {
        unlockSemaphore();
        printlnD(" - float already stored");
    }
}

float MemoryProviderInternal::loadDouble(String key, double defaultValue)
{
    lockSemaphore("loadDouble_" + key);
    if (_preferences->isKey(key.c_str()))
    {
        double d = _preferences->getDouble(key.c_str(), defaultValue);
        unlockSemaphore();
        return d;
    }
    else
    {
        unlockSemaphore();
        return defaultValue;
    }
}

void MemoryProviderInternal::removeKey(String key)
{
    printlnA("MP: Removing key: " + key + "\n");
    lockSemaphore("remove " + key);
    _preferences->remove(key.c_str());
    unlockSemaphore();
}

void MemoryProviderInternal::factoryReset()
{
    Serial.println("MemoryProvider -> factory reset...");
    if (_preferences != nullptr)
    {
        Serial.println("Preferences are ok");
    }
    else
    {
        Serial.println("Preferences are not ok");
    }
    lockSemaphore("factoryReset");
    _preferences->clear();
    unlockSemaphore();
    Serial.println("..Done");
}

void MemoryProviderInternal::lockSemaphore(String owner)
{
    Serial.println("Trying to lock by " + owner);
    if (_mutexOwner != "")
        Serial.println("Already locked from " + _mutexOwner);

    xSemaphoreTake(preferencesMutex, portMAX_DELAY);
    _mutexOwner = owner;
    Serial.println("Memory locked succesfully by " + owner);
    begin();
}

void MemoryProviderInternal::unlockSemaphore()
{
    Serial.println("Unlock memory semaphore from " + _mutexOwner);
    _mutexOwner = "";
    end();
    xSemaphoreGive(preferencesMutex);
}

#include "memory_provider_internal.h"
#include <Preferences.h>

#include <nvs.h>

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

MemoryProviderInternal::MemoryProviderInternal()
{
}

void MemoryProviderInternal::begin(String name = "vivarium")
{
    // printlnA("Initializing memory");
    _preferences = new Preferences();
    _preferences->begin(name.c_str(), false);
    nvs_stats_t nvs_stats;
    nvs_get_stats(NULL, &nvs_stats);
    // printA("Count: UsedEntries = ");
    // printlnA(nvs_stats.used_entries);
    // printA("FreeEntries = ");
    // printlnA(nvs_stats.free_entries);
    // printA("AllEntries = ");
    // printlnA(nvs_stats.total_entries);
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
}

void MemoryProviderInternal::end()
{
    _preferences->end();
}

bool MemoryProviderInternal::loadStruct(String key, void *value, size_t len)
{

    if (_preferences->isKey(key.c_str()))
    {
        size_t structlength = _preferences->getBytesLength(key.c_str());
        if (structlength != len)
        {
            printlnA("Error");
            printlnE("Struct has incorrect size");
            return false;
        }
        else
        {
            _preferences->getBytes(key.c_str(), value, len);
            debugA("Loaded struct under '%s' key", key.c_str());
            return true;
        }
    }
    else
    {
        debugA("No struct saved under '%s' key", key.c_str());
        return false;
    }
}

void MemoryProviderInternal::saveStruct(String key, const void *value, size_t len)
{
    printA("Saving struct under key ");
    printlnA(key);
    size_t bytes = _preferences->putBytes(key.c_str(), value, len);
    debugA("Saved %d bytes", bytes);
    _incrementWrites();
}

void MemoryProviderInternal::_incrementWrites()
{
    _writeCount++;
    printD("New number of writes = ");
    printD(_writeCount);
    printD(" (");
    printD(_preferences->putUInt(NUMBER_OF_WRITES_KEY, _writeCount));
    printlnD(" bytes)");
}

void MemoryProviderInternal::_incrementBytes(int bytes)
{
    _bytesWritten += bytes;
    printD("New number of bytes = ");
    printD(_bytesWritten);
    _preferences->putUInt(NUMBER_OF_BYTES_KEY, _bytesWritten);
}

void MemoryProviderInternal::saveString(String key, String value)
{
    printD("MP: Saving string ");
    printD(value);
    if (!_preferences->isKey(key.c_str()))
    {
        size_t bytes = _preferences->putString(key.c_str(), value);
        printD(" (");
        printD(bytes);
        printD(" bytes) under key ");
        printlnD(key);
    }
    else if (_preferences->getString(key.c_str(), String("")) != value)
    {
        size_t bytes = _preferences->putString(key.c_str(), value);
        printD(" (");
        printD(bytes);
        printD(" bytes) under key ");
        printlnD(key);
    }
    else
    {
        printlnD(" - string is already stored");
    }
}

String MemoryProviderInternal::loadString(String key, String defaultValue)
{
    if (_preferences->isKey(key.c_str()))
    {
        printlnA("NVS - key " + key + " exists");
        return _preferences->getString(key.c_str(), defaultValue);
    }
    else
    {
        printlnA("NVS - key " + key + " doesn't exist");
        return defaultValue;
    }
}

void MemoryProviderInternal::saveBool(String key, bool value)
{
    printD("MP: Saving bool ");
    printD(value ? "true" : "false");
    if (!_preferences->isKey(key.c_str()))
    {
        _preferences->putBool(key.c_str(), value);
    }
    else if (_preferences->getBool(key.c_str()) != value)
    {
        size_t bytes = _preferences->putBool(key.c_str(), value);
        printD(" (");
        printD(bytes);
        printD(" bytes) under key ");
        printlnD(key);
    }
}
bool MemoryProviderInternal::loadBool(String key, bool defaultValue)
{
    if (_preferences->isKey(key.c_str()))
    {
        return _preferences->getBool(key.c_str(), defaultValue);
    }
    else
    {
        return defaultValue;
    }
}

void MemoryProviderInternal::saveInt(String key, uint32_t value)
{
    printD("MP: Saving int ");
    printD(value);
    if (!_preferences->isKey(key.c_str()))
    {
        size_t bytes = _preferences->putUInt(key.c_str(), value);
        printD(" (");
        printD(bytes);
        printD(" bytes) under key ");
        printlnD(key);
    }
    else if (_preferences->getUInt(key.c_str(), value) != value)
    {
        size_t bytes = _preferences->putUInt(key.c_str(), value);
        printD(" (");
        printD(bytes);
        printD(" bytes) under key ");
        printlnD(key);
    }
    else
    {
        printlnD(" - int already stored");
    }
}
uint32_t MemoryProviderInternal::loadInt(String key, uint32_t defaultValue)
{
    if (_preferences->isKey(key.c_str()))
    {
        return _preferences->getUInt(key.c_str(), defaultValue);
    }
    else
    {
        return defaultValue;
    }
}

void MemoryProviderInternal::saveFloat(String key, float value)
{
    printD("MP: Saving float ");
    printD(value);
    if (!_preferences->isKey(key.c_str()))
    {
        size_t bytes = _preferences->putFloat(key.c_str(), value);
        printD(" (");
        printD(bytes);
        printD(" bytes) under key ");
        printlnD(key);
    }
    else if (_preferences->getFloat(key.c_str(), value) != value)
    {
        size_t bytes = _preferences->putFloat(key.c_str(), value);
        printD(" (");
        printD(bytes);
        printD(" bytes) under key ");
        printlnD(key);
    }
    else
    {
        printlnD(" - float already stored");
    }
}

float MemoryProviderInternal::loadFloat(String key, float defaultValue)
{
    if (_preferences->isKey(key.c_str()))
    {
        return _preferences->getFloat(key.c_str(), defaultValue);
    }
    else
    {
        return defaultValue;
    }
}

void MemoryProviderInternal::saveDouble(String key, double value)
{
    printD("MP: Saving double ");
    if (!_preferences->isKey(key.c_str()))
    {
        size_t bytes = _preferences->putDouble(key.c_str(), value);
        printD(" (");
        printD(bytes);
        printD(" bytes) under key ");
        printlnD(key);
    }
    else if (_preferences->getDouble(key.c_str(), value) != value)
    {
        size_t bytes = _preferences->putDouble(key.c_str(), value);
        printD(" (");
        printD(bytes);
        printD(" bytes) under key ");
        printlnD(key);
    }
    else
    {
        printlnD(" - float already stored");
    }
}

float MemoryProviderInternal::loadDouble(String key, double defaultValue)
{
    if (_preferences->isKey(key.c_str()))
    {
        return _preferences->getDouble(key.c_str(), defaultValue);
    }
    else
    {
        return defaultValue;
    }
}

void MemoryProviderInternal::removeKey(String key)
{
    printlnA("MP: Removing key: " + key + "\n");
    _preferences->remove(key.c_str());
}

void MemoryProviderInternal::factoryReset()
{
    _preferences->clear();
}
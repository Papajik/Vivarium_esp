#ifndef MEMORY_INTERNAL_H
#define MEMORY_INTERNAL_H

#include "memory_provider.h"

#define SETTINGS_KEY "settings"

#define STATE_KEY "state"

#define DEVICE_KEY "device"

#define NUMBER_OF_WRITES_KEY "writes"
#define NUMBER_OF_BYTES_KEY "bytes"

#define LCD_SETTINGS_KEY "lcd"
#define CLOCK_SETTINGS_KEY "clock"

class Preferences;

class MemoryProviderInternal : public MemoryProvider
{
public:
    MemoryProviderInternal();
    void begin(String name);
    void end();

    // Extern modules settings
    void saveStruct(String key, const void *, size_t);
    bool loadStruct(String key, void *, size_t);

    //Writes
    void saveString(String key, String value);
    String loadString(String key, String defaultValue);

    void saveBool(String key, bool value);
    bool loadBool(String key, bool defaultValue);

    void saveInt(String key, uint32_t value);
    uint32_t loadInt(String key, uint32_t defaultValue);

    void saveFloat(String key, float value);
    float loadFloat(String key, float defaultValue);

    void saveDouble(String key, double value);
    float loadDouble(String key, double defaultValue);

    void removeKey(String key);

    void factoryReset();

private:
    Preferences *_preferences;
    void _incrementWrites();
    void _incrementBytes(int bytes);
};
#endif
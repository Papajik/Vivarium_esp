#ifndef MEMORY_H
#define MEMORY_H

#include <HardwareSerial.h>

#define SETTINGS_KEY "settings"

#define STATE_KEY "state"

#define DEVICE_KEY "device"

#define NUMBER_OF_WRITES_KEY "writes"

#define LCD_SETTINGS_KEY "lcd"
#define CLOCK_SETTINGS_KEY "clock"

struct StateStruct;
class Preferences;

class MemoryProvider
{
public:
    MemoryProvider();
    void begin();

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

    unsigned int writeCount;
    unsigned int bytesWritten;

private:
    bool _memoryConnected;
    Preferences *_preferences;

    bool _settingsSaved();
    bool _userSaved();
    String _getStringFromMemory(String key);
    void _incerementWrites();
};

/*
Global instance of memory provider
*/
extern MemoryProvider memoryProvider;

#endif
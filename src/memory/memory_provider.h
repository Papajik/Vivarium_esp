#ifndef MEMORY_H
#define MEMORY_H

#include <HardwareSerial.h>

class MemoryProvider
{
public:
    MemoryProvider() {};
    virtual void begin(String name = "vivarium");
    virtual void end();

    // Extern modules settings
    virtual void saveStruct(String key, const void *, size_t);
    virtual bool loadStruct(String key, void *, size_t);

    //Writes
    virtual void saveString(String key, String value);
    virtual String loadString(String key, String defaultValue);

    virtual void saveBool(String key, bool value);
    virtual bool loadBool(String key, bool defaultValue);

    virtual void saveInt(String key, uint32_t value);
    virtual uint32_t loadInt(String key, uint32_t defaultValue);

    virtual void saveFloat(String key, float value);
    virtual float loadFloat(String key, float defaultValue);

    virtual void saveDouble(String key, double value);
    virtual float loadDouble(String key, double defaultValue);

    virtual void removeKey(String key);
    virtual void factoryReset();

    int getWriteCount() { return _writeCount; }
    int getBytesWritten() { return _bytesWritten; }

protected:
    unsigned int _writeCount;
    unsigned int _bytesWritten;
    virtual void _incrementWrites();
    virtual void _incrementBytes(int bytes);
};

#endif
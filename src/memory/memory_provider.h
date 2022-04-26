#ifndef MEMORY_H
#define MEMORY_H

#include <HardwareSerial.h>

/**
* @brief Interface of Memory Provider
Used to save data into NVS memory so it can be retrieved after power outage

* 
*/
class MemoryProvider
{
public:
    MemoryProvider(){};
    virtual ~MemoryProvider(){};
    /**
    * @brief Init memory, sets name of storage
    * 
    * @param name 
    */
    virtual void init(String name = "vivarium") = 0;

    // Extern modules settings
    virtual void saveStruct(String key, const void *, size_t) = 0;
    virtual bool loadStruct(String key, void *, size_t) = 0;

    //Writes
    virtual void saveString(String key, String value) = 0;
    virtual String loadString(String key, String defaultValue) = 0;

    virtual void saveBool(String key, bool value) = 0;
    virtual bool loadBool(String key, bool defaultValue) = 0;

    virtual void saveInt(String key, uint32_t value) = 0;
    virtual uint32_t loadInt(String key, uint32_t defaultValue) = 0;

    virtual void saveFloat(String key, float value) = 0;
    virtual float loadFloat(String key, float defaultValue) = 0;

    virtual void saveDouble(String key, double value) = 0;
    virtual float loadDouble(String key, double defaultValue) = 0;

    virtual void removeKey(String key) = 0;
    /**
    * @brief Clear every record inside NVS
    * 
    */
    virtual void factoryReset() = 0;

    int getWriteCount() { return _writeCount; }
    int getBytesWritten() { return _bytesWritten; }

protected:
    virtual void begin() = 0;
    virtual void end() = 0;
    unsigned int _writeCount = 0;
    unsigned int _bytesWritten = 0;
    virtual void _incrementWrites();
    virtual void _incrementBytes(int bytes);
};

#endif
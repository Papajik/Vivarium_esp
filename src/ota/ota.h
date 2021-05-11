#ifndef _OTA_SERVICE_H
#define _OTA_SERVICE_H

#define OTA_IDLE 1
#define OTA_CANCEL -1
#define OTA_COMPLETED 2
#define OTA_UPDATING 0

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

class MemoryProvider;

class OtaService
{
public:
    OtaService(MemoryProvider *);
    void begin();
    bool isNewVersion(String);
    bool prepareAndStartUpdate(String, String);
    int onLoop();
    bool isFirmwareUpdating();

private:
    void startUpdate();

    MemoryProvider *_memoryProvider;

    unsigned long _lastWrite;
    String _firmwareVersion;
    String _newFirmwareVersion;

    bool _firmwareUpdateRunning = false;
};

extern OtaService *otaService;

#endif
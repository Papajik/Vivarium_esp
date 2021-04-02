#ifndef _OTA_SERVICE_H
#define _OTA_SERVICE_H

#define OTA_IDLE 1
#define OTA_CANCEL -1
#define OTA_COMPLETED 2
#define OTA_UPDATING 0

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

class OtaService
{
public:
    OtaService();
    void begin();
    void parseNewFirmwareVersion(String);
    void startUpdate();
    int onLoop();
    bool isFirmwareUpdating();

private:
    unsigned long _lastWrite;
    String _firmwareVersion;
    String _newFirmwareVersion;

    bool _firmwareUpdateRunning = false;
};

extern OtaService otaService;

#endif
/**
* @file ota.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/

#ifndef _OTA_SERVICE_H
#define _OTA_SERVICE_H

#define OTA_IDLE 1
#define OTA_CANCEL -1
#define OTA_FAIL -2
#define OTA_COMPLETED 2
#define OTA_UPDATING 0

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

class MemoryProvider;
class TextOutput;

/**
* @brief OTA service is used for remote FW updating
* 
*/
class OtaService
{
public:
    OtaService(MemoryProvider *, TextOutput *);
    void begin();
    bool isNewVersion(String);
    bool prepareAndStartUpdate(String, String);
    int onLoop();
    bool isFirmwareUpdating();
    bool failCallback();

private:
    void startUpdate();

    MemoryProvider *_memoryProvider;
    TextOutput *_textOutput;
    int updateTextCount = 0;

    unsigned long _lastWrite = 0;
    String _firmwareVersion = "";
    String _newFirmwareVersion = "";

    bool _firmwareUpdateRunning = false;
};

extern OtaService *otaService;

#endif
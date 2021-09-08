#include "ota.h"
#include "../memory/memory_provider.h"
#include "../modules/internal/lcd_display/textOutput.h"

#include "../firebase/firebase.h"
#include "WiFi.h"
#include "../firebase/cert.h"
#include "../bluetooth/bluetooth.h"
#include <HttpsOTAUpdate.h>
#include <WiFiClientSecure.h>

#define FIRMWARE_VERSION_KEY "fw_ver"
#define FIRMWARE_NEW_VERSION_KEY "fw_ver_new"
#define FIRMWARE_DOWNLOAD_URL_KEY "fw_name"
#define WRITE_DELAY 1000

static String downloadUrl = "";

OtaService *otaService;

void HttpEvent(HttpEvent_t *event)
{
  switch (event->event_id)
  {
  case HTTP_EVENT_ERROR:
    printlnA("Http Event Error");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    printlnA("Http Event On Connected");
    break;
  case HTTP_EVENT_HEADER_SENT:
    printlnA("Http Event Header Sent");
    break;
  case HTTP_EVENT_ON_HEADER:
    Serial.printf("Http Event On Header, key=%s, value=%s\n", event->header_key, event->header_value);
    break;
  case HTTP_EVENT_ON_DATA:
    break;
  case HTTP_EVENT_ON_FINISH:
    printlnA("Http Event On Finish");
    break;
  case HTTP_EVENT_DISCONNECTED:
    printlnA("Http Event Disconnected");
    break;
  }
}

OtaService::OtaService(MemoryProvider *memoryProvider, TextOutput *output) : _memoryProvider(memoryProvider), _textOutput(output)
{
  printlnA("OTA SERVICE - created");
  HttpsOTA.onHttpEvent(HttpEvent);
}

void OtaService::begin()
{
  printlnA("OTA SERVICE - begin");
  _firmwareVersion = _memoryProvider->loadString(FIRMWARE_VERSION_KEY, "0");
  _newFirmwareVersion = _memoryProvider->loadString(FIRMWARE_NEW_VERSION_KEY, "0");
  printlnA("Current version = " + _firmwareVersion);
  printlnA("Available version = " + _newFirmwareVersion);

  if (_firmwareVersion != _newFirmwareVersion)
  {
    downloadUrl = _memoryProvider->loadString(FIRMWARE_DOWNLOAD_URL_KEY, "");

    printA("New firmware download url found:");
    printlnA(downloadUrl);
    startUpdate();
  }
}

bool OtaService::isNewVersion(String version)
{
  return _firmwareVersion != version;
}

bool OtaService::prepareAndStartUpdate(String downloadUrl, String version)
{
  if (!downloadUrl.isEmpty() && !version.isEmpty())
  {
    _newFirmwareVersion = version;
    _memoryProvider->saveString(FIRMWARE_NEW_VERSION_KEY, version);
    _memoryProvider->saveString(FIRMWARE_DOWNLOAD_URL_KEY, String(downloadUrl));
    startUpdate();
    return true;
  }
  return false;
}

void OtaService::startUpdate()
{
  _textOutput->setText({"FW update", _newFirmwareVersion});
  _firmwareUpdateRunning = true;
  HttpsOTA.begin(downloadUrl.c_str(), https_cert, false);
}

int OtaService::onLoop()
{
  //printlnA("OTA - on LOOP");
  if (_firmwareUpdateRunning)
  {
    HttpsOTAStatus_t otaStatus = HttpsOTA.status();

    // Restart ESP on successfull update
    if (otaStatus == HTTPS_OTA_SUCCESS)
    {
      _textOutput->setText({"FW update", "DONE"}, 2000);
      printlnA("Firmware updated. Rebooting device");
      _memoryProvider->saveString(FIRMWARE_VERSION_KEY, _newFirmwareVersion);
      return OTA_COMPLETED;
    }
    else if (otaStatus == HTTPS_OTA_FAIL)
    {

      _textOutput->setText({"FW update", "FAILED"}, 2000);
      printlnE("Firmware upgrade Fail");
      _firmwareUpdateRunning = false;
      return OTA_FAIL;
    }
    else if (otaStatus == HTTPS_OTA_UPDATING)
    {
      if (millis() > _lastWrite + WRITE_DELAY)
      {
        updateTextCount++;
        if (updateTextCount > 3)
          updateTextCount = 0;
        switch (updateTextCount)
        {
        case 0:
          _textOutput->setText({"FW update", _newFirmwareVersion});
          break;
        case 1:
          _textOutput->setText({"FW update.", _newFirmwareVersion});
          break;
        case 2:
          _textOutput->setText({"FW update..", _newFirmwareVersion});
          break;
        case 3:
        default:
          _textOutput->setText({"FW update...", _newFirmwareVersion});
        }
        _lastWrite = millis();
        printlnA("*");
      }
      return OTA_UPDATING;
    }
    else if (otaStatus == HTTPS_OTA_ERR)
    {
      printlnA("HTTPS_OTA_ERR");
      _firmwareUpdateRunning = false;
      return OTA_CANCEL;
    }
  }
  return OTA_IDLE;
}

bool OtaService::isFirmwareUpdating()
{
  return _firmwareUpdateRunning;
}

bool OtaService::failCallback()
{
  int fail = _memoryProvider->loadInt("FW_FAIL_COUNT", 0);
  if (fail < 5)
  {
    _memoryProvider->saveInt("FW_FAIL_COUNT", ++fail);
    return false;
  }
  else
  {
    _memoryProvider->saveInt("FW_FAIL_COUNT", 0);
    _memoryProvider->saveString(FIRMWARE_VERSION_KEY, "0");
    _memoryProvider->saveString(FIRMWARE_NEW_VERSION_KEY, "0");
    _memoryProvider->saveString(FIRMWARE_DOWNLOAD_URL_KEY, "");
    return true;
  }
}
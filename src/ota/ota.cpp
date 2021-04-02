#include "ota.h"
#include "../memory/memory_provider.h"

#include "../firebase/firebase.h"
#include "WiFi.h"
#include "../firebase/cert.h"
#include "../bluetooth/bluetooth.h"
#include <HttpsOTAUpdate.h>
#include <WiFiClientSecure.h>

#define FIRMWARE_VERSION_KEY "fw_ver"
#define FIRMWARE_NEW_VERSION_KEY "fw_ver_new"
#define FIRMWARE_DOWNLOAD_URL_KEY "fw_name"
#define FIRMWARE_URL "https://firebasestorage.googleapis.com/v0/b/vivarium-control-unit.appspot.com/o/firmware%2F"
#define WRITE_DELAY 1000

static String downloadUrl = "";

OtaService otaService;

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

OtaService::OtaService()
{
  printlnA("OTA SERVICE - created");
  HttpsOTA.onHttpEvent(HttpEvent);
}

void OtaService::begin()
{
  printlnA("OTA SERVICE - begin");
  _firmwareVersion = memoryProvider.loadString(FIRMWARE_VERSION_KEY, "0");
  _newFirmwareVersion = memoryProvider.loadString(FIRMWARE_NEW_VERSION_KEY, "0");
  printlnA("Current version = " + _firmwareVersion);
  printlnA("Available version = " + _newFirmwareVersion);

  if (_firmwareVersion != _newFirmwareVersion)
  {
    downloadUrl = memoryProvider.loadString(FIRMWARE_DOWNLOAD_URL_KEY, "");

    printA("New firmware download url found:");
    printlnA(downloadUrl);
    startUpdate();
  }
}

/**
 * @brief Parses firmware version. If version is new and has valid url, then proceeds to update.
 * 
 *  
 * If version is same as previous, nothing happens. 
 * If new version is available but URL not, logs and error. 
 * If new version and download URL are available, pause Firebase and Bluetooth, then proceeds to update.
 * 
 * @param version Version from Firebase Stream
 */
void OtaService::parseNewFirmwareVersion(String version)
{
  if (_firmwareVersion != version)
  {
    _newFirmwareVersion = version;
    printlnA("NEW FIRMWARE AVAILABLE");
    printlnD("Current version = " + _firmwareVersion);
    printlnD("Available version = " + _newFirmwareVersion);
    downloadUrl = firebaseService.getFirmwareDownloadUrl(version);
    if (downloadUrl != "")
    {
      // save new version and name of its file, then proceed to update
      memoryProvider.saveString(FIRMWARE_NEW_VERSION_KEY, version);
      memoryProvider.saveString(FIRMWARE_DOWNLOAD_URL_KEY, downloadUrl);
      printlnD("New firmware file = " + downloadUrl);
      firebaseService.stopFirebase();
      bleController.stop();
      startUpdate();
    }
    else
    {
      printlnE("New firmware URL not ready, abort update");
    }
  }
}

void OtaService::startUpdate()
{

  if (downloadUrl != "")
  {
    printlnA("Starting to upgrade");
    printA("Url = ");
    printlnA(downloadUrl);
    _firmwareUpdateRunning = true;
    HttpsOTA.begin(downloadUrl.c_str(), cert, false);
  }
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
      printlnA("Firmware updated. Rebooting device");
      memoryProvider.saveString(FIRMWARE_VERSION_KEY, _newFirmwareVersion);
      return OTA_COMPLETED;
    }
    else if (otaStatus == HTTPS_OTA_FAIL)
    {
      printlnE("Firmware upgrade Fail");
      _firmwareUpdateRunning = false;
      return OTA_CANCEL;
    }
    else if (otaStatus == HTTPS_OTA_UPDATING)
    {
      if (millis() > _lastWrite + WRITE_DELAY)
      {
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

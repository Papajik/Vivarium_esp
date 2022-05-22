#include "rtc.h"
#include <ESP32Time.h>
#include "RTClib.h"
#include <TimeAlarms.h>
#include <esp.h>
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#define NTP_SERVER "pool.ntp.org"
#define NTP_SERVER_EUROPE "europe.pool.ntp.org"
#define NTP_SERVER_BACKUP "time.nist.gov"

#define GMT_OFFSET_SEC 3600 //+1
// #define DST_OFFSET_SEC 3600 //Daylight offset
#define DST_OFFSET_SEC 3600 //Daylight offset

RTC_DS1307 rtc_ds1307;

RTC rtc;

RTC::RTC() {}
void RTC::begin()
{
    rtc_ds1307.begin();
}
bool RTC::isRunning() { return rtc_ds1307.isrunning(); }
void RTC::saveCurrentTime()
{

    printlnA("Saving time to RTC");
    ESP32Time esp32Time;
    rtc_ds1307.adjust(DateTime(esp32Time.getYear(), esp32Time.getMonth(), esp32Time.getDay(),
                               esp32Time.getHour(), esp32Time.getMinute(), esp32Time.getSecond()));
    printlnA(esp32Time.getDateTime());
}
void RTC::loadTimeFromRTC()
{

    if (rtc.isRunning())
    {
        printlnA("Loading time from RTC");
        DateTime now = rtc_ds1307.now();
        ESP32Time esp32Time;
        esp32Time.setTime(now.second(), now.minute(), now.hour(), now.day(), now.month(), now.year());
        debugA("%2d:%2d:%2d %d. %d. %4d", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year())
    }
    else
    {
        printlnE("RTC is not running");
    }
}

bool RTC::syncNTP()
{
    printlnA("Sync time with NTP server");
    configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, NTP_SERVER_EUROPE, NTP_SERVER, NTP_SERVER_BACKUP);
    Alarm.configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC);

    tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        printlnE("Syncing Failed");
        return false;
    }
    else
    {
        printA("Time synchronized - ");
        printlnA(String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min));
        return true;
    }
}
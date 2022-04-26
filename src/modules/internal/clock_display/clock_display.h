/**
* @file clock_display.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef _CLOCK_DISPLAY_H_
#define _CLOCK_DISPLAY_H_

#include <Arduino.h>

#define DEFAULT_CLOCK_BRIGHTNESS 8
#define DEFAULT_CLOCK_REFRESH_INTERVAL 10000 //refresh every 10 seconds to display more accurate time

#define MIN_INTERVAL_CONSTRAINT 1000
#define MAX_INTERVAL_CONSTRAINT 60000

#define DIO_PIN 17
#define CLK_PIN 16

class millisDelay;
class RobotDyn4DigitDisplay;

/*
Provides 4 digit segment display control

refreshDisplay() has to be called from loop()

Uses esp32 getLocalTime() function to get synchronized time from ntp server
*/
class ClockDisplay
{
public:
    ClockDisplay(int clk, int dio);
    ~ClockDisplay();

    /*
    @Brief Initializes 4DigitDisplay, sets brigthness and starts delay to refresh display
    */
    void begin();

    /*
    @brief Checks if delay passed and refresh clock display if so. Should be called from loop()
    */
    void refreshDisplay();

    /*
    @brief Sets new refresh interval in ms. Minimal interval is 1000 ms, maximum should be 60000 ms. 
    There is risk of skipping minute on clock otherwise 
    @param interval Refresh interval in [ms]
    */
    void setRefreshInterval(unsigned long ms);

private:
    void displayInvalid();
    bool showTime();
    RobotDyn4DigitDisplay *_clock;
    millisDelay *_delay;
    tm timeinfo;
};

struct ClockSettings
{
    uint32_t brigthness;
    uint32_t refresh_interval;
};

extern ClockDisplay clockDisplay;
extern ClockSettings clockSettings;

#endif
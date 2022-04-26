#ifndef V_RTC_H
#define V_RTC_H



class RTC
{
public:
    RTC();
    void begin();
    bool isRunning();
    bool syncNTP();
    void saveCurrentTime();
    void loadTimeFromRTC();
};

extern RTC rtc;

#endif
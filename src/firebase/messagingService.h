#ifndef MESSAGING_SERVICE_H
#define MESSAGING_SERVICE_H

#include <map>
#include <vector>
#include <WString.h>
enum FCM_TYPE
{
    CONNECTION,
    CROSS_LIMIT
};

class MessagingService
{
public:
    MessagingService(){};
    void loadSettings();
    void sendFCM(String, String, FCM_TYPE, String);
    void addToken(String);

    void setDelayFCM(int);
    void setNotifyOnConnectionChange(bool);
    void setNotifyOnCrossLimit(bool);
    void setDistinctNotification(bool);

private:
    //settings
    int _delayFCMNotification = 0;
    bool _notifyOnConnectionChange = false;
    bool _notifyOnCrossLimit = false;
    bool _distinctNotifications = false;

    std::map<String, unsigned long> _lastValueSendTimeMap;
    unsigned long _lastValueSendTime = 0;
    std::vector<String> _tokens;
};

extern MessagingService messagingService;

#endif
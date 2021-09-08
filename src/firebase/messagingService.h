#ifndef MESSAGING_SERVICE_H
#define MESSAGING_SERVICE_H

#include <map>
#include <vector>
#include <WString.h>

enum FCM_TYPE
{
    CONNECTION,
    CROSS_LIMIT,
    TRIGGER
};

class Auth;
class FirebaseJson;

class MessagingService
{
    int _delayFCMNotification = 0;
    bool _notifyOnConnectionChange = false;
    bool _notifyOnCrossLimit = false;
    bool _triggerNotitifacitons = false;

public:
    MessagingService(Auth *);

    void init();

    void getSettings();
    void getTokens();
    void parseTokens(FirebaseJson *);

    void clearTokens();
    void sendFCM(String, String, FCM_TYPE, String, bool silent = false);
    void addToken(String);

    void setDelayFCM(int);
    void setNotifyOnConnectionChange(bool);
    void setNotifyOnCrossLimit(bool);
    void setTriggerNotifications(bool);

private:
    void sendMessage(String title, String body, String tag, String token, bool timePrefix, bool silent, bool legacy = true);

    Auth *_auth;
    //settings

    std::map<String, unsigned long> _lastValueSendTimeMap;
    std::vector<String> _tokens;
};

#endif
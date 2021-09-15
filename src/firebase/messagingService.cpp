#include "messagingService.h"
#include "firebase.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#include <Firebase_ESP_Client.h>

#include "semaphore/firebaseSemaphore.h"
#include "firebaseBdo.h"
#include "../utils/timeHelper.h"

#include "../auth/auth.h"

#include "sender/sender.h"

#define FCM_SERVER_KEY "AAAA2l0CYos:APA91bHTNr_Xf2_aW6-NYot7tamMXBt9Q7jkjFpJzTKPAW8oJ7GzDtpHGEwM7GRPTHwasiQD9UXOKuxloTxNuAxBcHFrEtft1gGHDwCZPEWcz9Tiv8GxHpadJRUXpnB5Bol8QLhHzJRi"

MessagingService::MessagingService(Auth *auth) : _auth(auth)
{
    // Legacy FCM //TODO test and remove legacy server key
    Firebase.FCM.setServerKey(FCM_SERVER_KEY);
}

void MessagingService::setTriggerNotifications(bool b)
{
    _triggerNotitifacitons = b;
}

void MessagingService::setDelayFCM(int i)
{
    printlnA(i);
    printlnA(_delayFCMNotification);
    _delayFCMNotification = i;
}

void MessagingService::setNotifyOnConnectionChange(bool b)
{
    _notifyOnConnectionChange = b;
}
void MessagingService::setNotifyOnCrossLimit(bool b)
{
    printlnA(b ? "True" : "False");
    _notifyOnCrossLimit = b;
}

void MessagingService::sendFCM(String title, String body, FCM_TYPE type, String moduleTag, bool silent)
{
    printlnA("SendFCM");
    printlnA(title);
    printlnA(body);
    if (!Firebase.ready())
    {
        printlnW("Firebase is not ready, FCM can't be sent");
        return;
    }
    // Check rules for value notification

    if (type == FCM_TYPE::CROSS_LIMIT)
    {

        if (!_notifyOnCrossLimit)
        {
            printlnA("FCM value cross limit prohibited");
            return;
        }

        if (millis() - _lastValueSendTimeMap[moduleTag] < _delayFCMNotification)
        {
            printlnA("FCM - not enough delay");
            // Can not send value yet
            return;
        }
        else
        {
            _lastValueSendTimeMap[moduleTag] = millis();
        }
    }

    // Check rules for connection

    if (type == FCM_TYPE::CONNECTION && !_notifyOnConnectionChange)
    {
        printlnI("FCM connection change prohibited");
        return;
    }

    if (type == FCM_TYPE::TRIGGER && !_triggerNotitifacitons)
    {
        printlnI("FCM Trigger notifications prohibited");
        return;
    }
    parseMessage(title, body, moduleTag, silent, true);
}

void MessagingService::parseMessage(String title, String body, String tag, bool silent, bool timePrefix)
{
    // Add time preffix
    if (timePrefix)
    {
        time_t now;
        time(&now);
        tm *time = localtime(&now);
        String tmp = formatTime(time->tm_hour, time->tm_min);
        body = tmp + String(" - ") + body;
    }

    printlnA("Proceed to send a token");
    printlnA(body);
    firebaseSender.addMessage(std::make_shared<Message>(Message(title.c_str(), body.c_str(), tag.c_str(), silent)));
}

void MessagingService::init()
{
    getSettings();
    getTokens();
}

void MessagingService::getSettings()
{
    printlnA("Get FCM Settings");
    String path = "/users/" + _auth->getUserId() + "/settings";
    firebaseSemaphore.lockSemaphore("getSettings");
    if (Firebase.RTDB.getJSON(firebaseBdo, path.c_str()))
    {
        printlnA("FCM Settings recieved");
        printlnA(firebaseBdo->dataType());
        printlnA(firebaseBdo->jsonString());

        FirebaseJson &json = firebaseBdo->jsonObject();
        FirebaseJsonData data;

        if (json.get(data, "/nLimit", false))
        {
            setNotifyOnCrossLimit(data.boolValue);
        }

        if (json.get(data, "/nDelay", false))
        {
            setDelayFCM(data.intValue);
        }

        if (json.get(data, "/nConn", false))
        {
            setNotifyOnConnectionChange(data.boolValue);
        }

        if (json.get(data, "/nTrigg", false))
        {
            setTriggerNotifications(data.boolValue);
        }

        /// Refresh tokens
        if (json.get(data, "/tokens", false))
        {
        }
    }
    else
    {
        printlnA("Couldnt receive FCM settings");
    }
    firebaseSemaphore.unlockSemaphore();
}

void MessagingService::getTokens()
{
    printlnA("Get FCM Tokens");
    String path = "/users/" + _auth->getUserId() + "/tokens";
    firebaseSemaphore.lockSemaphore("getTokens");
    if (Firebase.RTDB.getJSON(firebaseBdo, path.c_str()))
    {
        firebaseSender.clearTokens();
        firebaseSender.pauseMessages();
        FirebaseJson tokenJson = firebaseBdo->jsonObject();
        parseTokens(&tokenJson);
        firebaseSender.continueMessages();
    }
    else
    {
        printlnA("Couldnt receive tokens");
    }
    firebaseSemaphore.unlockSemaphore();
}

void MessagingService::parseTokens(FirebaseJson *json)
{
    String buffer;
    json->toString(buffer, true);
    printlnA("Pretty printed JSON data:");
    printlnA(buffer);
    size_t len = json->iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
        json->iteratorGet(i, type, key, value);
        printA(", Key: ");
        printA(key);
        printA(", Value: ");
        printlnA(value);
        if (value == "true")
        { //If key is valid then add token
            firebaseSender.addFCMToken(key);
        }
    }
    json->iteratorEnd();
}
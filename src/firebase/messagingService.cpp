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

#define FCM_SERVER_KEY "AAAA2l0CYos:APA91bHTNr_Xf2_aW6-NYot7tamMXBt9Q7jkjFpJzTKPAW8oJ7GzDtpHGEwM7GRPTHwasiQD9UXOKuxloTxNuAxBcHFrEtft1gGHDwCZPEWcz9Tiv8GxHpadJRUXpnB5Bol8QLhHzJRi"
#define FCM_SILENT_CHANNEL "vivarium_silent"

MessagingService::MessagingService(Auth *auth) : _auth(auth)
{
    _tokens.reserve(5);
    // Legacy FCM
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

void MessagingService::addToken(String s)
{
    _tokens.push_back(s);
}

void MessagingService::sendFCM(String title, String body, FCM_TYPE type, String moduleTag, bool silent)
{
    printlnA("SendFCM");
    printlnA(title);
    printlnA(body);
    printlnD((int)_tokens.size());

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

    printlnI("Proceed to send");

    for (String token : _tokens)
    {
        printlnA("Proceed to send a token");
        sendMessage(title, body, moduleTag, token, silent, true);
    }
    printlnI("All sent");
}

void MessagingService::sendMessage(String title, String body, String tag, String token, bool silent, bool timePrefix, bool legacy)
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
    if (legacy)
    {
        FCM_Legacy_HTTP_Message msg;

        msg.payloads.notification.title = title.c_str();
        msg.payloads.notification.body = body.c_str();
        msg.payloads.notification.tag = tag.c_str();
        msg.targets.to = token.c_str();
        if (silent)
        {
            msg.payloads.notification.android_channel_id = FCM_SILENT_CHANNEL;
        }
        firebaseSemaphore.lockSemaphore("sendFCM");
        if (Firebase.FCM.send(firebaseBdo, &msg)) //send message to recipient
        {
            printlnA("PASSED");
            printlnA(Firebase.FCM.payload(firebaseBdo));
            printlnA("------------------------------------");
            printlnA();
        }
        else
        {
            printlnA("FAILED");
            printlnA("REASON: " + firebaseBdo->errorReason());
            printlnA("------------------------------------");
            printlnA();
        }
        firebaseSemaphore.unlockSemaphore();
    }
    else
    {
        FCM_HTTPv1_JSON_Message msg;
        msg.notification.title = title.c_str();
        msg.notification.body = body.c_str();
        msg.android.notification.tag = tag.c_str();
        if (silent)
        {
            msg.android.notification.channel_id = FCM_SILENT_CHANNEL;
        }
        msg.token = token.c_str();
        firebaseSemaphore.lockSemaphore("sendFCM");
        if (Firebase.FCM.send(firebaseBdo, &msg)) //send message to recipient
        {
            printlnA("PASSED");
            printlnA(Firebase.FCM.payload(firebaseBdo));
            printlnA("------------------------------------");
            printlnA();
        }
        else
        {
            printlnA("FAILED");
            printlnA("REASON: " + firebaseBdo->errorReason());
            printlnA("------------------------------------");
            printlnA();
        }
        firebaseSemaphore.unlockSemaphore();
    }
}

void MessagingService::clearTokens()
{
    _tokens.clear();
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
        clearTokens();
        FirebaseJson tokenJson = firebaseBdo->jsonObject();
        parseTokens(&tokenJson);
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
            addToken(key);
        }
    }
    json->iteratorEnd();
}
#include "messagingService.h"
#include "firebase.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#include <Firebase_ESP_Client.h>


#include "semaphore/firebaseSemaphore.h"
#include "firebaseBdo.h"

MessagingService::MessagingService()
{
}

void MessagingService::setDistinctNotification(bool b)
{
    _distinctNotifications = b;
}

void MessagingService::setDelayFCM(int i)
{
    _delayFCMNotification = i;
}

void MessagingService::setNotifyOnConnectionChange(bool b)
{
    _notifyOnConnectionChange = b;
}
void MessagingService::setNotifyOnCrossLimit(bool b)
{
    _notifyOnCrossLimit = b;
}

void MessagingService::addToken(String s)
{
    _tokens.push_back(s);
}

void MessagingService::sendFCM(String title, String body, FCM_TYPE type, String moduleTag)
{
    printlnI("SendFCM");
    printlnI(title);
    printlnI(body);
    printlnD((int)_tokens.size());

    // Check rules for value notification

    if (type == FCM_TYPE::CROSS_LIMIT)
    {

        if (!_notifyOnCrossLimit)
        {
            printlnA("FCM value cross limit prohibited");
            return;
        }

        if (_distinctNotifications)
        {
            // Distinct notifications
            if (millis() - _lastValueSendTimeMap[moduleTag] < _delayFCMNotification)
            {
                printlnA("FCM - not enough delay");
                // Can not send value yet
                return;
            }
            else
            {
                // FCM is possible
                _lastValueSendTime = millis();
                _lastValueSendTimeMap[moduleTag] = millis();
            }
        }
        else
        {
            // One notification per all modules

            if (millis() - _lastValueSendTime < _delayFCMNotification)
            {
                return;
            }
            else
            {
                _lastValueSendTime = millis();
                _lastValueSendTimeMap[moduleTag] = millis();
            }
        }
    }

    // Check rules for connection

    if (type == FCM_TYPE::CONNECTION && !_notifyOnConnectionChange)
    {
        printlnI("FCM connection change prohibited");
        return;
    }

    printlnI("Proceed to send");

    for (String token : _tokens)
    {
        printlnA("Proceed to send a token");
        sendMessage(title, body, token, true);
    }
    printlnI("All sent");
}

void MessagingService::sendMessage(String title, String body, String token, bool timePrefix)
{
    // Add time preffix
    if (timePrefix)
    {
        time_t now;
        time(&now);
        tm *time = localtime(&now);
        String hour = time->tm_hour < 10 ? "0" + String(time->tm_hour) : String(time->tm_hour);
        String minute = time->tm_min < 10 ? "0" + String(time->tm_min) : String(time->tm_min);

        String timePreffix = hour + ":" + minute + " - ";
        body = timePreffix + body;
    }

    printlnA("Proceed to send a token");
    FCM_HTTPv1_JSON_Message msg;
    msg.notification.title = title.c_str();
    msg.notification.body = body.c_str();
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
        // TODO check if removed
        printlnA();
    }
    firebaseSemaphore.unlockSemaphore();
}

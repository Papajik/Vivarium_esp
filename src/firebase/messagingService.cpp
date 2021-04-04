#include "messagingService.h"
#include "firebase.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

MessagingService messagingService;

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
    printlnA("SendFCM");
    printlnA(title);
    printlnA(body);
    printlnV((int)_tokens.size());

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
        printlnA("FCM connection change prohibited");
        return;
    }

    printlnA("Proceed to send");

    for (String token : _tokens)
    {
        printlnA("Proceed to send a token");
        firebaseService.sendFCM(title, body, token, true);
    }
}

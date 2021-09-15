#include "sender.h"
#include "../semaphore/firebaseSemaphore.h"
#include "../firebaseBdo.h"

#include <Firebase_ESP_Client.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#define FCM_SILENT_CHANNEL "vivarium_silent"
FirebaseSender firebaseSender;

void senderTask(void *parameter)
{
    for (;;)
    {
        firebaseSender.checkQueue();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void FirebaseSender::checkQueue()
{
    if (_sendMessages && Firebase.ready())
    {
        xSemaphoreTake(_messageMutex, portMAX_DELAY);
        if (!_messages.empty())
        {
            printlnA("Messages: " + _messages.size());
            message = _messages.front();
        }
        else
        {
            message = nullptr;
        }
        xSemaphoreGive(_messageMutex);
        if (message != nullptr)
        {
            for (int i = 0; i < _fcmTokens.size(); i++)
            {
                sendMessage(message, _fcmTokens.at(i));
            }
        }

        xSemaphoreTake(_messageMutex, portMAX_DELAY);
        if (!_messages.empty())
        {
            _messages.pop();
        }
        xSemaphoreGive(_messageMutex);
    }
}

FirebaseSender::FirebaseSender()
{
    _messageMutex = xSemaphoreCreateMutex();
    xSemaphoreGive(_messageMutex);
}

FirebaseSender::~FirebaseSender() { xSemaphoreGive(_messageMutex); }

void FirebaseSender::addFCMToken(String token)
{
    printlnA("Adding token: ");
    printlnA(token);
    _fcmTokens.push_back(token);
}

void FirebaseSender::start()
{
    xTaskCreate(
        senderTask,   /* Task function. */
        "senderTask", /* String with name of task. */
        5200,         /* Stack size in bytes */
        NULL,         /* Parameter passed as input of the task */
        1,            /* Priority of the task.  */
        NULL);        /* Task handle. */
}

void FirebaseSender::addMessage(std::shared_ptr<Message> message)
{
    xSemaphoreTake(_messageMutex, portMAX_DELAY);
    _messages.push(message);
    xSemaphoreGive(_messageMutex);
}

void FirebaseSender::sendMessage(std::shared_ptr<Message> message, String token)
{
    printlnA("Sending message");
    printlnA(token);
    FCM_HTTPv1_JSON_Message msg;
    msg.notification.title = message->title.c_str();
    msg.notification.body = message->body.c_str();
    msg.android.notification.tag = message->tag.c_str();
    if (message->silent)
    {
        msg.android.notification.channel_id = FCM_SILENT_CHANNEL;
    }
    msg.token = token.c_str();
    firebaseSemaphore.lockSemaphore("sendMessage");
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

void FirebaseSender::clearTokens()
{
    _fcmTokens.clear();
}

void FirebaseSender::pauseMessages()
{
    _sendMessages = false;
}
void FirebaseSender::continueMessages()
{
    _sendMessages = true;
}
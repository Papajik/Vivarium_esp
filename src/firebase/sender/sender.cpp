#include "sender.h"
#include "../semaphore/firebaseSemaphore.h"
#include "../firebaseBdo.h"
#include "../../auth/auth.h"
#include "../firebase.h"

#include <Firebase_ESP_Client.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#define FCM_SILENT_CHANNEL "vivarium_silent"

#include <esp_task_wdt.h>
#define PING_ALIVE_INTERVAL 60000 // 1 minute

FirebaseSender firebaseSender;

void senderTask(void *parameter)
{
    esp_task_wdt_init(20, true); // extend watchdog for 20 seconds with reset enabled
    for (;;)
    {
        if (firebaseService != nullptr && firebaseService->isRunning())
        {
            firebaseSender.checkJson();
            vTaskDelay(10 / portTICK_PERIOD_MS);
            firebaseSender.checkMessages();
            vTaskDelay(10 / portTICK_PERIOD_MS);
            firebaseSender.checkDataQueue();
            vTaskDelay(10 / portTICK_PERIOD_MS);
            firebaseSender.pingAlive();
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void FirebaseSender::pingAlive()
{

    if (millis() > _lastAlive + PING_ALIVE_INTERVAL)
    {
        _lastAlive = millis();
        time_t now;
        time(&now);
        uploadToDatabase("/devices/" + std::string(_auth->getDeviceId().c_str()) + "/info/lastAlive", (long long)now);
        sendJson = false;
    }
}

FirebaseSender::FirebaseSender()
{
    _messageMutex = xSemaphoreCreateMutex();
    _dataMutex = xSemaphoreCreateMutex();
    xSemaphoreGive(_messageMutex);
    xSemaphoreGive(_dataMutex);
}

FirebaseSender::~FirebaseSender()
{
    xSemaphoreGive(_messageMutex);
    xSemaphoreGive(_dataMutex);
}

void FirebaseSender::start()
{
    xTaskCreate(
        senderTask,   /* Task function. */
        "senderTask", /* String with name of task. */
        6500,         /* Stack size in bytes */
        NULL,         /* Parameter passed as input of the task */
        1,            /* Priority of the task.  */
        NULL);        /* Task handle. */
}

void FirebaseSender::setAuth(Auth *a)
{
    _auth = a;
}

/// JSON
void FirebaseSender::checkJson()
{
    if (sendJson && Firebase.ready())
    {
        time_t now;
        time(&now);
        char time[40];
        ultoa(now, time, 10);
        uploadToDatabase("/sensorData/" + std::string(_auth->getDeviceId().c_str()) + "/" + std::string(time), &json);
        sendJson = false;
        json.clear();
    }
}

/// RTDB
void FirebaseSender::addData(std::shared_ptr<Data> data)
{
    xSemaphoreTake(_dataMutex, portMAX_DELAY);
    _dataQueue.push(data);
    xSemaphoreGive(_dataMutex);
}
void FirebaseSender::checkDataQueue()
{
    if (Firebase.ready())
    {

        xSemaphoreTake(_dataMutex, portMAX_DELAY);
        if (!_dataQueue.empty())
        {
            printlnA("Data in queue: " + String(_dataQueue.size()));
            _data = _dataQueue.front();
            _dataQueue.pop();
        }
        xSemaphoreGive(_dataMutex);
        if (_data != nullptr)
        {
            printlnA("Sending data");
            sendData(_data);
            _data = nullptr;
        }
    }
}

void FirebaseSender::sendData(std::shared_ptr<Data> data)
{

    switch (data->type)
    {
    case DataType::BOOL:
        uploadToDatabase(data->path, data->boolData);
        break;
    case DataType::STRING:
        uploadToDatabase(data->path, data->stringData);
        break;
    case DataType::FLOAT:
        uploadToDatabase(data->path, data->floatData);
        break;
    case DataType::INT:
        uploadToDatabase(data->path, data->intData);
        break;
    case DataType::UNKNOWN:
        break;
    default:
        break;
    }
}

template <typename T2>
void FirebaseSender::uploadToDatabase(std::string path, T2 value)
{
    firebaseSemaphore.lockSemaphore("uploadToDatabase");
    debugA("Upload to database..%s", Firebase.RTDB.set(firebaseBdo, path.c_str(), value) ? "Ok" : firebaseBdo->errorReason().c_str());
    firebaseSemaphore.unlockSemaphore();
}

/// Messages

void FirebaseSender::checkMessages()
{
    if (_sendMessages && Firebase.ready())
    {

        xSemaphoreTake(_messageMutex, portMAX_DELAY);
        if (!_messages.empty())
        {
            printlnA("Messages in queue: " + String(_messages.size()));
            message = _messages.front();
            _messages.pop();
        }
        xSemaphoreGive(_messageMutex);
        if (message != nullptr)
        {
            for (int i = 0; i < _fcmTokens.size(); i++)
            {
                sendMessage(message, _fcmTokens.at(i));
            }
            message = nullptr;
        }
    }
}

void FirebaseSender::addFCMToken(String token)
{
    _fcmTokens.push_back(token);
}

void FirebaseSender::addMessage(std::shared_ptr<Message> message)
{
    xSemaphoreTake(_messageMutex, portMAX_DELAY);
    _messages.push(message);
    xSemaphoreGive(_messageMutex);
}

void FirebaseSender::sendMessage(std::shared_ptr<Message> message, String token)
{
    printlnD(token);
    if (_useLegacyAPI)
    {
        FCM_Legacy_HTTP_Message msg;

        msg.payloads.notification.title = message->title.c_str();
        msg.payloads.notification.body = message->body.c_str();
        msg.payloads.notification.tag = message->tag.c_str();
        msg.targets.to = token.c_str();
        if (message->silent)
        {
            msg.payloads.notification.android_channel_id = FCM_SILENT_CHANNEL;
        }
        firebaseSemaphore.lockSemaphore("sendMessage");
        debugA("Sending message... %s\n",
               Firebase.FCM.send(firebaseBdo, &msg)
                   ? "ok"
                   : firebaseBdo->errorReason().c_str());
        firebaseSemaphore.unlockSemaphore();
    }
    else
    {
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
        debugA("Sending message... %s\n",
               Firebase.FCM.send(firebaseBdo, &msg)
                   ? "ok"
                   : firebaseBdo->errorReason().c_str());
        firebaseSemaphore.unlockSemaphore();
    }
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

void FirebaseSender::useLegacyAPI(bool b)
{
    _useLegacyAPI = b;
}
/**
* @file sender.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/

#ifndef _F_SENDER_H
#define _F_SENDER_H

#include <FreeRTOS.h>
#include <freertos/semphr.h>
#include <queue>
#include <memory>
#include <WString.h>
#include <Firebase_ESP_Client.h>

class Auth;

struct Message
{
    const std::string title = "";
    const std::string body = "";
    const std::string tag = "";
    const bool silent = false;
    Message(const char *t, const char *b, const char *tag, bool silent) : title(t), body(b), tag(tag), silent(silent) {}
};

enum DataType
{
    INT,
    STRING,
    FLOAT,
    BOOL,
    UNKNOWN
};

struct Data
{
    std::string path = "";
    float floatData = 0;
    bool boolData = false;
    int intData = 0;
    std::string stringData = "";
    DataType type = DataType::UNKNOWN;
};


/**
* @brief  Service used to push data into Firebase
*   Handles both RTDB and FCM
*/
class FirebaseSender
{
public:
    FirebaseSender();
    ~FirebaseSender();
    void start();
    void setAuth(Auth *);

    // Messaging
    void addFCMToken(String);
    void addMessage(std::shared_ptr<Message>);
    void checkMessages();
    void clearTokens();
    void pauseMessages();
    void continueMessages();
    void useLegacyAPI(bool);

    // RTDB
    void addData(std::shared_ptr<Data>);
    void checkDataQueue();

    // JSON
    void checkJson();
    FirebaseJson json;
    bool sendJson = false;

    // Alive
    void pingAlive();

private:
    Auth *_auth;

    // Messaging
    void sendMessage(std::shared_ptr<Message>, String);
    std::vector<String> _fcmTokens;
    std::shared_ptr<Message> message = nullptr;
    std::queue<std::shared_ptr<Message>> _messages;
    SemaphoreHandle_t _messageMutex;
    bool _sendMessages = true;
    bool _useLegacyAPI = false;

    // RTDB
    void sendData(std::shared_ptr<Data>);
    template <typename T>
    void uploadToDatabase(std::string, T);

    std::queue<std::shared_ptr<Data>> _dataQueue;
    std::shared_ptr<Data> _data;
    SemaphoreHandle_t _dataMutex;

    // Alive
    unsigned long _lastAlive = 0;
};

extern FirebaseSender firebaseSender;

#endif
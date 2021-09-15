#ifndef _F_SENDER_H
#define _F_SENDER_H

#include <FreeRTOS.h>
#include <freertos/semphr.h>
#include <queue>
#include <memory>
#include <WString.h>

struct Message
{
    const std::string title = "";
    const std::string body = "";
    const std::string tag = "";
    const bool silent = false;
    Message(const char *t, const char *b, const char *tag, bool silent) : title(t), body(b), tag(tag), silent(silent) {}
};

class FirebaseSender
{
public:
    FirebaseSender();
    ~FirebaseSender();
    void addFCMToken(String);
    void start();
    void addMessage(std::shared_ptr<Message>);
    void checkQueue();
    void clearTokens();
    void pauseMessages();
    void continueMessages();

private:
    void sendMessage(std::shared_ptr<Message>, String);
    std::vector<String> _fcmTokens;
    std::shared_ptr<Message> message = nullptr;
    std::queue<std::shared_ptr<Message>> _messages;
    SemaphoreHandle_t _messageMutex;
    bool _sendMessages = true;
};

extern FirebaseSender firebaseSender;

#endif
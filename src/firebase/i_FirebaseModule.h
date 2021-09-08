#ifndef _I_FIREBASE_MODULE_H_
#define _I_FIREBASE_MODULE_H_

#include <Arduino.h>
#include "firebase.h"
#include "messagingService.h"
#include <Firebase_ESP_Client.h>

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#define PREFIX_SETTINGS "/settings"
#define PREFIX_STATE "/state"

class IFirebaseModule
{
public:
    IFirebaseModule(){};
    virtual ~IFirebaseModule(){};

    virtual void parseJson(FirebaseJson *, String) = 0;
    virtual String getSettingKey() = 0;
    virtual void parseValue(String key, String value) = 0;
    virtual bool updateSensorData(FirebaseJson *) = 0;
    void addFirebaseService(FirebaseService *service) { firebaseService = service; };
    void addMessagingService(MessagingService *service) { messagingService = service; };

protected:
    FirebaseService *firebaseService = nullptr;
    MessagingService *messagingService = nullptr;

    void sendConnectionChangeNotification(String module, bool connected)
    {
        String title = "Module " + module;
        if (messagingService != nullptr)
            messagingService->sendFCM(title, connected ? "Module connected " : "Module disconnected ", FCM_TYPE::CONNECTION, module);
    }
};

#endif
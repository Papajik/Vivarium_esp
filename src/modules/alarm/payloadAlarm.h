#ifndef _PAYLOAD_ALARM_H
#define _PAYLOAD_ALARM_H

#include "baseAlarm.h"

template <typename K>
class PayloadAlarm : public BaseAlarm<PayloadTrigger<K>>
{
public:
    PayloadAlarm(TriggerCallback callback, MemoryProvider *provider, std::string prefix);
    void createTrigger(int time, String key, K payload);
    bool getTriggerPayload(uint8_t, K *payload);
    bool getNextTriggerPayload(K *payload);

    // Memory
    virtual void saveTriggerToNVS(std::shared_ptr<PayloadTrigger<K>>); // ok
    virtual void loadTriggerFromNVS(int index);                        // ok
    virtual void printTrigger(std::shared_ptr<PayloadTrigger<K>>);     // ok

    // Firebase
    virtual void createNewTriggersFromJson(FirebaseJson *); // ok

    virtual void createTriggerFromJson(FirebaseJson *json, String triggerKey);
    virtual bool updateTriggerFromJson(std::shared_ptr<PayloadTrigger<K>> trigger, FirebaseJson *json); // ok
    virtual void parseTriggerCustomValue(std::shared_ptr<PayloadTrigger<K>>, String key, String value);
    // ********
    //
    // Module
    //
    // ********

    virtual bool getPayloadFromJson(FirebaseJson *, K &) = 0;
    virtual bool getPayloadFromValue(String key, String value, K &payload) = 0;
};

#endif

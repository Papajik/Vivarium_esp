#ifndef _V_ALARM_H
#define _V_ALARM_H

#include "baseAlarm.h"
#include <string>
// class MemoryProvider;
class PlainAlarm : public BaseAlarm<Trigger>
{
public:
    PlainAlarm(TriggerCallback callback, MemoryProvider *, std::string);
    void createTrigger(int time, String key);

protected:
    virtual void saveTriggerToNVS(std::shared_ptr<Trigger>);                                      // ok
    virtual void printTrigger(std::shared_ptr<Trigger>);                                          // ok
    virtual void loadTriggerFromNVS(int index);                                                   // ok
    virtual void createNewTriggersFromJson(FirebaseJson *);                                       // ok
    virtual void parseTriggerCustomValue(std::shared_ptr<Trigger>, String jsonKey, String value); // ok
    virtual bool updateTriggerFromJson(std::shared_ptr<Trigger> trigger, FirebaseJson *json);     // ok
    virtual void createTriggerFromJson(FirebaseJson *json, String triggerKey);
};

#endif

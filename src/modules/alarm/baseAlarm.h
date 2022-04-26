/**
* @file baseAlarm.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-09-23
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef _BASE_ALARM_H
#define _BASE_ALARM_H

#include <TimeAlarms.h>
#include <WString.h>
#include <map>
#include <memory>
#include <FreeRTOS.h>
#include <freertos/semphr.h>
#include <string>

#define INVALID_MEMORY_ID -1
#define MAX_TRIGGERS 10

typedef void (*TriggerCallback)();
class MemoryProvider;
class FirebaseJson;
class FirebaseJsonData;

struct Trigger
{
    int storageId = INVALID_MEMORY_ID;
    int hour;
    int minute;
    AlarmId id;
    String firebaseKey;
};

template <typename T>
struct PayloadTrigger : public Trigger
{
    T payload{};
};

/**
* @brief Template class of trigger alarm. Can be used when there is need to change some value at given time
* 
* @tparam T 
*/
template <typename T>
class BaseAlarm
{
public:
    BaseAlarm<T>(TriggerCallback callback, MemoryProvider *provider, std::string memoryPrefix);
    ~BaseAlarm();
    std::shared_ptr<T> getNextTrigger();
    bool getNextTriggerTime(int *);
    void clearTriggers();
    int getTriggersCount();

    /**
    * @brief Removes trigger based on firebaseKey
    * 
    * @param firebaseKey 
    */
    void removeTrigger(String firebaseKey);

    void lockSemaphore(std::string);
    void unlockSemaphore();
    std::map<String, std::shared_ptr<T>> _triggers;
    TriggerCallback getCallback();

    void loadTriggersFromNVS();
    /**
    * @brief Get available memory ID. Returned ID is reserved!
    * 
    * @return int 
    */
    int getAvailableMemoryId();
    void freeMemoryId(int id);

    /**
    * @brief Parses JSON into Alarm Triggers. 
    * Edits existing, removes obsolete and creates new ones.
    * 
    * 
    */
    void parseTriggersJson(FirebaseJson *);
    void parseTriggerJson(FirebaseJson *, String);

    /**
    * @brief Parses Trigger value from firebase callback.
    * Calls parseTriggerCustomValue() or delete triggers completely based on given key and value
    * 
    * @param firebaseKey 
    * @param value 
    */
    void parseTriggerValue(String firebaseKey, String value);

    void printTriggers();

    /**
    * @brief Updates trigger time
    * 
    * @param time 
    * @return true On time updated
    * @return false On no change
    */
    bool updateTriggerTime(std::shared_ptr<T>, int time);

    std::shared_ptr<T> findTrigger(String key);

    // ********************
    //
    // Alarm implementation
    //
    // ********************
    virtual void saveTriggerToNVS(std::shared_ptr<T>) = 0;
    virtual bool loadTriggerFromNVS(int index) = 0;
    virtual void printTrigger(std::shared_ptr<T>) = 0;

    /**
    * @brief  Creates multiple triggers From JSON object. 
    * 
    * 
    * @param json 
    */
    virtual void createNewTriggersFromJson(FirebaseJson *json) = 0;

    /**
    * @brief Creates a Trigger From JSON object. 
    * 
    */
    virtual void createTriggerFromJson(FirebaseJson *json, String triggerKey) = 0;

    // *********************
    //
    // Module implementation
    //
    // *********************

    /**
    * @brief   Update Trigger from JSON. Checks if JSON contains all necessary data - time and optionally payload. 
    * JSON is If Json doesn't have all data, no change occurs
    * 
    * @param trigger 
    * @param json 
    * @return true on trigger update
    * @return false on no change
    */
    virtual bool updateTriggerFromJson(std::shared_ptr<T> trigger, FirebaseJson *json) = 0;

    virtual void parseTriggerCustomValue(std::shared_ptr<T>, String key, String value) = 0; // ok
protected:
    MemoryProvider *_provider = nullptr;
    TriggerCallback _callback;
    std::string _memoryPrefix = "";

private:
    bool availableIds[MAX_TRIGGERS];

    SemaphoreHandle_t triggersMutex;
    std::string _owner = "";

    std::shared_ptr<T> _lastTrigger;
};
#endif

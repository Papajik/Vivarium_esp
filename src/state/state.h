/*!
* \file c:\Users\Papi\Documents\Arduino\Vivarium_esp\src\state\state.h
* \author Papaj Michal <papaj.mich@gmail.com>
* \version 0.1
* \date 02/04/2021
* \brief 
* \remarks None
* 
* 
* 
*/


#ifndef _STATE_H_
#define _STATE_H_
/*! Importation of librairies*/
#include <Arduino.h>

#include <map>
// #include <vector>
#include "helper.h"

class ChangeCallback;

union item_value
{
  uint32_t i;
  float f;
  bool b;
};

enum item_type
{
  type_uint32,
  type_float,
  type_bool,
};

class ChangeCallback
{
public:
  virtual void callback(item_value) = 0;
};

typedef void (*UpdateTimeCallback)(time_t time);


template <typename T>
struct state_item
{
  ChangeCallback *changeCallback;
  T value;
};

/**
Storage for vivarium states. Using key-value map with additional callback on value change.

  Key - String
  Value - item_type (uint32_t, float, bool)
  Callback - void f(item_value)

  Callback is triggered when value is changed. Trigger has to be defined. 


* @brief Storage for vivarium states. Using key-value map
*/
class StateStorage
{
private:
  std::map<String, state_item<uint32_t>> uint32t_items;
  std::map<String, state_item<float>> float_items;
  std::map<String, state_item<bool>> bool_items;

  UpdateTimeCallback updateTimeCallback;
  time_t last_update;
  void updateTime();

public:
  StateStorage();
  ~StateStorage();

  /*
  @overload
  @brief Add new item if key is unique or change its value if key already exists 
  @param key
  @param value
  @return 0 on new item. 1 otherwise 
  */
  int setValue(String, uint32_t);

  /*
  @overload
  @brief Add new item if key is unique or change its value if key already exists 
  @param key
  @param value
  @return 0 on new item. 1 otherwise 
  */
  int setValue(String, float);

  /*
  @overload
  @brief Add new item if key is unique or change its value if key already exists 
  @param key
  @param value
  @return 0 on new item. 1 otherwise 
  */
  int setValue(String, bool);

  /*
  @overload
  @brief Gets uint32_t value from storage
  @param key 
  @return true if found,  false  otherwise
  */
  bool getValue(String key, uint32_t *value);

  /*
  @overload
  @brief Gets uint32_t value from storage
  @param key 
  @return true if found,  false  otherwise
  */
  bool getValue(String key, float *value);

  /*
  @overload
  @brief Gets uint32_t value from storage
  @param key 
  @return true if found,  false  otherwise
  */
  bool getValue(String key, bool *value);

  /*
  @overload
  @brief Add new item if key is unique or change its value if key already exists 
  @param key
  @param value
  @param callback Callback to be called on item change
  @return 0 on new item. 1 otherwise 
  */
  int setValueWithCallback(String, uint32_t, ChangeCallback *);

  /*
  @overload
  @brief Add new item if key is unique or change its value if key already exists 
  @param key
  @param value
  @param callback Callback to be called on item change
  @return 0 on new item. 1 otherwise 
  */
  int setValueWithCallback(String, float, ChangeCallback *);

  /*
  @overload
  @brief Add new item if key is unique or change its value if key already exists 
  @param key
  @param value
  @param callback Callback to be called on item change
  @return 0 on new item. 1 otherwise 
  */
  int setValueWithCallback(String, bool, ChangeCallback *);

  /*
  @brief Set callback of already stored item
  @param key Key of stored item
  @param callback Callback to be called on item change
  @param type Type of saved  item
  @return 0 on new item, 1 on stored item, -1 on wrong item_type
  */
  int setCallback(String, ChangeCallback *, item_type);

  /*
  @brief Removes stored item under given key
  @param key Key of stored item
  @param type Type of stored item
  @return 1 on success delete. 0 if key is not found. -1 on unsupported item_type
  */
  int removeUint32t(String, item_type);

  /*
  */
  void printState();

  void setUpdateTimeCallback(UpdateTimeCallback callback);

 // void getStateJson(JsonVariant);

  time_t getLastUpdate();
};

extern StateStorage stateStorage;

#endif

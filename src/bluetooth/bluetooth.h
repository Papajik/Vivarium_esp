/*!
* \file c:\Users\Papi\Documents\Arduino\Vivarium_esp\src\bluetooth\bluetooth.h
* \author Papaj Michal <papaj.mich@gmail.com>
* \version 0.1
* \date 02/04/2021
* \brief 
* \remarks None
* 
* 
* 
*/

#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

/*! Importation of librairies*/
#include <vector>
#include <WString.h>
#include "i_bluetooth_pin.h"

#define BLUETOOTH_BUTTON 7

/*!
* \def DEFAULT_BLE_NAME
* Description
*/
#define DEFAULT_BLE_NAME "Aquarium service"
/*!
* \def BLE_NAME_KEY
* Description
*/
#define BLE_NAME_KEY "ble_name"

/*!
* \def SERVICE_CREDENTIALS_UUID
* Description
*/
#define SERVICE_CREDENTIALS_UUID "E553C32A-64A0-11EB-AE93-0242AC130002"
/*!
* \def SERVICE_STATE_UUID
* Description
*/
#define SERVICE_STATE_UUID "24A54BE9-44C0-43AB-BF07-5F4D7AF751D0"
/*!
* \def SERVICE_SETTINGS_UUID
* Description
*/
#define SERVICE_SETTINGS_UUID "2B1CBE3F-1A9F-460C-8040-8240590EDB4A"

/*!
* \def CHARACTERISTIC_BLE_NAME
* Description
*/
#define CHARACTERISTIC_BLE_NAME "56600001-BE5D-4370-877B-C4A2ACE639E8"

/*!
* \def CREDENTIALS_ID
* Description
*/
#define CREDENTIALS_ID 1
/*!
* \def STATE_ID
* Description
*/
#define STATE_ID 2
/*!
* \def SETTINGS_ID
* Description
*/
#define SETTINGS_ID 3

class IBluetooth;
class NimBLEServer;
class MemoryProvider;
class LedControl;

/**
 * @brief Bluetooth Controller
 * 
 */
class BLEController
{
public:
  BLEController(MemoryProvider *, LedControl *);

  /*
  @brief Starts bluetooth server. 
  It is recomended to add PINHandler on active security.
  */
  void init();

  /*
  @brief Add interface pointer which is used to handle displaying bluetooth PIN during connection
  */
  void addBluetoothPINHandler(IBluetoothPIN *);

  /*
  @brief Restarts advertising after device disconnect
  */
  void restartAdvertising();

  /*
  @brief Adds external module with BLE support. Modules need to be added before "setupServices()" is called.
  */
  void addModule(IBluetooth *);

  void checkBluetooth();

  bool isDeviceConnected();
  void setDeviceConnected(bool);

  void setConnectionHandle(uint16_t);

  IBluetoothPIN *bluetoothPINHandler;

  void setBleName(String name);

  bool isRunning();

  void stop();

  void setStartInFuture();
  void setStopInFuture();
  void checkStop();

private:
  MemoryProvider *_memoryProvider;
  LedControl *_ledControl;

  bool _toStart = false;
  bool _toStop = false;

  bool _running = false;
  bool _initialized = false;
  /*
  @brief Iterates through all added modules and setup their services
  */
  void setupModuleServices();

  void callDisconnectOnModules();

  bool isDeviceConnecting();
  bool isDeviceDisconnecting();

  void onDeviceConnecting();
  void onDeviceDisconnecting();

  std::vector<IBluetooth *> _modules;

  NimBLEServer *pServer = nullptr;

  uint16_t conn_handle = 0;

  bool _deviceConnected = false;
  bool _oldDeviceConnected = false;

  String _bluetoothName = "";
  bool _nameChanged = false;
  bool _secured = false;
};

extern BLEController *bleController;

#endif
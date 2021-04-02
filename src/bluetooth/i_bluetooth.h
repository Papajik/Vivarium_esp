#ifndef _I_BLE_H_
#define _I_BLE_H_

#include <NimBLECharacteristic.h>

class NimBLEService;
class NimBLECharacteristic;
class IModule;

float parseFloat(NimBLECharacteristic *, float);
double parseDouble(NimBLECharacteristic *, double);

/**
 * @brief Interface for Bluetooth module
 * 
 */
class IBluetooth
{
public:
    IBluetooth(){};
    virtual ~IBluetooth(){};

    /**
      * @brief Set credential characteristic for this module
      * 
      * @param settings Service to which credential characteristics will be binded
      */
    virtual void setupBLECredentials(NimBLEService *credentials){};

    /**
      * @brief Set setting characteristic for this module
      * 
      * @param settings Service to which setting characteristics will be binded
      */
    virtual void setupBLESettings(NimBLEService *settings){};

    /**
      * @brief Set state characteristic for this module
      * 
      * @param settings Service to which state characteristics will be binded
      */
    virtual void setupBLEState(NimBLEService *state){};

    /**
     * @brief Callback after Bluetooth device is disconnected
     * 
     */
    virtual void onBLEDisconnect() = 0;

    /**
     * @brief Callback after new connection is set
     * 
     */
    virtual void onBLEConnect() = 0;

    /**
     * @brief Get count of hendles per service
     * 
     * This function is needed to determine how much memory needs to be allocated for bluetooth characteristics per service
     * 
     * @param settings pointer to number of setting handles
     * @param state pointer to number of state handles
     * @param credentials pointer to number of credential handles
     */

    virtual void getHandlesCount(int *settings, int *state, int *credentials) = 0;

protected:
    bool isBluetoothRunning();
    void setConnectionCallback(NimBLEService *, const char *, IModule *);
    NimBLECharacteristic *setSettingsCharacteristic(NimBLEService *, const char *, NimBLECharacteristicCallbacks *);
    NimBLECharacteristic *setStateCharacteristic(NimBLEService *, const char *, NimBLECharacteristicCallbacks *);

    NimBLECharacteristic *_connectedCharacteristic;
};

class IsModuleConnectedCallbacks : public NimBLECharacteristicCallbacks
{
public:
    IsModuleConnectedCallbacks(IModule *);

private:
    void onWrite(NimBLECharacteristic *);
    void onRead(NimBLECharacteristic *);
    IModule *module;
};

#endif
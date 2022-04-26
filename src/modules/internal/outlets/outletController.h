/**
* @file outletController.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef _OUTLET_CONTROLER_H_
#define _OUTLET_CONTROLER_H_

#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"

#include <vector>

class MemoryProvider;

class OutletController : public IFirebaseModule, public IBluetooth
{
public:
    OutletController(MemoryProvider *, int);

    void setOutlet(int, bool);
    bool isOutletOn(int);

    void onLoop();

    /// Firebase
    virtual void parseJson(FirebaseJson *, String);
    virtual String getSettingKey();
    virtual void parseValue(String key, String value);
    virtual bool updateSensorData(FirebaseJson *);

    /// Bluetooth
    virtual void setupBLESettings(NimBLEService *settings);
    virtual void setupBLEState(NimBLEService *state);

    virtual void onBLEDisconnect();
    virtual void onBLEConnect();
    virtual void getHandlesCount(int *settings, int *state, int *credentials);

    bool reserveOutlet(int);

private:
    void updateOutletState();

    MemoryProvider *_memoryProvider;
    int _outletCount;
    std::vector<bool> _reserved;
    std::vector<bool> _outlets;
    std::vector<bool> _outletChanged;
};

extern OutletController *outletController;

#endif
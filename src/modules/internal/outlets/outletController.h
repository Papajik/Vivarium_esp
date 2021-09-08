#ifndef _OUTLET_CONTROLER_H_
#define _OUTLET_CONTROLER_H_

#include "../../../firebase/i_FirebaseModule.h"
#include "../../../bluetooth/i_bluetooth.h"

#include <vector>

// #define OUTLET_COUNT 2

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

    // bool _reserved[OUTLET_COUNT] = {false};
    // bool _outlets[OUTLET_COUNT] = {false};
    // bool _outletChanged[OUTLET_COUNT] = {false};
};

extern OutletController *outletController;

#endif
#ifndef _OUTLET_CONTROLER_H_
#define _OUTLET_CONTROLER_H_

#include "../../../firebase/i_FirebaseModule.h"

#define OUTLET_COUNT 2

class MemoryProvider;

class OutletController : public IFirebaseModule
{
public:
    OutletController(MemoryProvider *);

    void setOutlet(int, bool);
    bool isOutletOn(int);

    void onLoop();

    /// Firebase
    virtual void parseJson(FirebaseJson *, String);
    virtual String getSettingKey();
    virtual void parseValue(String key, String value);
    virtual void updateSensorData(FirebaseJson *);

private:
    void updateOutletState();

    MemoryProvider *_memoryProvider;
    bool _outlets[OUTLET_COUNT] = {false};
    bool _outletChanged[OUTLET_COUNT] = {false};
};

extern OutletController *outletController;

#endif
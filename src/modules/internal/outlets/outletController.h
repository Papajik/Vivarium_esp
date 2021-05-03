#ifndef _OUTLET_CONTROLER_H_
#define _OUTLET_CONTROLER_H_

#include "../../../firebase/i_FirebaseModule.h"

#define OUTLET_COUNT 2

class OutletController : public IFirebaseModule
{
public:
    OutletController();
    void begin();

    void setOutlet(int, bool);
    bool isOutletOn(int);

    void onLoop();


    /// Firebase
    virtual void parseJson(FirebaseJson *, String);
    virtual String getSettingKey();
    virtual void parseValue(String key, String value);
    virtual void updateSensorData(FirebaseJson *);



private:
    bool _outlets[OUTLET_COUNT];
    bool _outletChanged[OUTLET_COUNT];
};

extern OutletController outletController;

#endif
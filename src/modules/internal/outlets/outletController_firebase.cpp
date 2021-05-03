#include "outletController.h"

#define SETTINGS_OUTLET_KEY "outlet"
#define KEY_OUTLET_1 "/outlet/o1"

void OutletController::parseJson(FirebaseJson *json, String path)
{
    printlnA("OutletController - parseJson");
    printlnV("Path: " + path);

    FirebaseJsonData jsonData;
    if (json->get(jsonData, KEY_OUTLET_1, false))
    {
        setOutlet(1, jsonData.boolValue);
    }
}

String OutletController::getSettingKey()
{
    printlnA("outletCOntroller - returning key");
    return SETTINGS_OUTLET_KEY;
}

void OutletController::parseValue(String key, String value)
{
    printlnA("OutletController parse value");
    printV("key = ");
    printlnV(key);
    printV("value = ");
    printlnV(value);

    if (key.equals(PREFIX_SETTINGS + String(KEY_OUTLET_1)))
    {
        if (value == "true")
        {
            setOutlet(1, true);
        }
        else
        {
            setOutlet(1, false);
        }
    }
}

void OutletController::updateSensorData(FirebaseJson *) {}
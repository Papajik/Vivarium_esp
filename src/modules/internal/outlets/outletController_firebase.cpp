#include "outletController.h"

#define SETTINGS_OUTLET_KEY "outlet"
#define KEY_OUTLET "/outlet/o"

void OutletController::parseJson(FirebaseJson *json, String path)
{
    printlnA("OutletController - parseJson");
    printlnV("Path: " + path);

    FirebaseJsonData jsonData;
    for (int i = 0; i < _outletCount; i++)
    {

        if (json->get(jsonData, KEY_OUTLET + String(i), false))
        {
            setOutlet(i, jsonData.boolValue);
        }
    }
}

String OutletController::getSettingKey()
{
    return SETTINGS_OUTLET_KEY;
}

void OutletController::parseValue(String key, String value)
{
    printlnA("OutletController parse value");
    printV("key = ");
    printlnV(key);
    printV("value = ");
    printlnV(value);

    for (int i = 0; i < _outletCount; i++)
    {
        if (key.equals(PREFIX_STATE + String(KEY_OUTLET) + String(i)))
        {
            if (value == "true")
            {
                setOutlet(i, true);
            }
            else
            {
                setOutlet(i, false);
            }
        }
    }
}

bool OutletController::updateSensorData(FirebaseJson *json)
{
    return false;
}
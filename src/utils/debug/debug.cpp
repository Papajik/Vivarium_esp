#include "debug.h"
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include "../../memory/memory_provider.h"
#include "../../wifi/wifiProvider.h"
#include "../../bluetooth/bluetooth.h"
#include "memory.h"
#include "../../moduleControl/moduleControl.h"
#include "../../modules/internal/lcd_display/lcd.h"
#include "../../modules/external/led/led.h"
#include "../../modules/external/heater/heater.h"
#include "../../modules/external/feeder/feeder.h"
#include "../../modules/external/water_temperature/water_temp.h"
#include "../../modules/external/water_temperature/state_values.h"
#include "../../state/state.h"
#include "../../auth/auth.h"
#include "../../vivarium/version.h"
#include "../../vivarium/vivarium.h"
#include <TimeAlarms.h>
#include <time.h>
#include "mockTextModules.h"

MemoryProvider *provider;
ModuleControl *mControl;
FirebaseService *firebase;
Auth *auth;
WaterTempModule *wt;
Vivarium *v;

int mock_screen = 0;
std::vector<TextModule *> _modules;
int mock_count = 9;

int dDelay = 0;

void Debugger::addFirebase(FirebaseService *f)
{
    firebase = f;
}

void Debugger::addModuleControl(ModuleControl *mc)
{
    mControl = mc;
}

void Debugger::addAuth(Auth *a)
{
    auth = a;
}

void Debugger::addWaterTemp(WaterTempModule *w)
{
    wt = w;
}

void Debugger::addVivarium(Vivarium *viv)
{
    v = viv;
}

void Debugger::debugDelay()
{

    if (dDelay != 0)
    {
        unsigned long start = millis();
        while (start + dDelay > millis())
        {
            debugHandle();
        }
    }
}

void restart()
{
    if (v)
        v->restart();
}

void factoryReset()
{
    if (provider != nullptr)
    {
        provider->factoryReset();
        ESP.restart();
    }
}

void getTime()
{
    printA("Time = ");
    printlnA(time(nullptr));
}

void Debugger::addProvider(MemoryProvider *p)
{
    provider = p;
}

struct Test
{
    uint32_t t;
};

void testMemory()
{
    Serial.println("Testing bool");
    provider->saveBool("blabla", true);
    bool b = provider->loadBool("blabla", false);
    Serial.println(b);
    Test t;
    t.t = 12312;
    provider->saveStruct("str", &t, sizeof(Test));
    provider->factoryReset();
}

void setTemperature(String temp)
{
    Serial.println("Setting water temp temp to " + temp);
    if (wt)
    {
        wt->writeTemp(temp.toFloat());
    }
    else
    {
        stateStorage.setValue(STATE_WATER_TEMPERATURE, temp.toFloat());
    }
    stateStorage.setValue(STATE_WATER_TEMP_CONNECTED, true);
    heaterPtr->setConnected(true, true);
}

void setLoopDelay(String d)
{
    dDelay = d.toInt();
}

void printHeater()
{
    if (heaterPtr != nullptr)
    {
        debugA("Heater goal = %.2f", heaterPtr->getGoal());
        debugA("Heater power = %.2f", heaterPtr->getCurrentPower());
        heaterPtr->printPidSettings();
    }
}

void printTriggers()
{
    printlnA("Alarms:");
    Alarm.printAlarms();
    printlnA("\nLED: ");
    ledModulePtr->printTriggers();
    printlnA("\nHeater: ");
    heaterPtr->printTriggers();
    printlnA("\nFeeder: ");
    feederPtr->printTriggers();
}

void printState()
{
    char ptrTaskList[550];
    vTaskList(ptrTaskList);
    Serial.println("****************************************************");
    Serial.println("Task          State   Prio    Stack    Num    Core");
    Serial.println("****************************************************");
    Serial.println(ptrTaskList);
    Serial.println("****************************************************");
    printMemory();
}

void printFirmwareVersion()
{
    printlnA("VERSION: " + String(VIVARIUM_FIRMWARE_VERSION));
    printlnA("ID: " + auth->getDeviceId());
}

void switchMockDisplay()
{

    while (mock_screen != _modules.size())
    {
        lcdDisplay.displayText(_modules.at(mock_screen++)->getText());
        Serial.println("Printing text");
        Alarm.delay(3000);
        debugHandle();
    }
}

void printCredentials()
{
    wifiProvider->printCredentials();
}

void setWiFiPass(String pass)
{
    wifiProvider->setPassphrase(pass);
}

void setWiFiSsid(String ssid)
{
    wifiProvider->setSsid(ssid);
}

void setUserId(String userId)
{
    auth->setUserId(userId);
}

void switchBluetoth(int bluetooth)
{
    if (bluetooth == 1)
    {
        Serial.println("Bluetooth OFF");
        if (!mControl->isModuleConnected(BLUETOOTH_BUTTON))
        {
            mControl->buttonPressed(BLUETOOTH_BUTTON);
        }
        Serial.println("Bluetooth ON");
        bleController->setStopInFuture();
        firebase->setStartInFuture();
    }
    else
    {
        Serial.println("Bluetooth OFF");
        if (mControl->isModuleConnected(BLUETOOTH_BUTTON))
        {
            mControl->buttonPressed(BLUETOOTH_BUTTON);
        }

        firebase->setStopInFuture();
        bleController->setStartInFuture();
    }
}

void removeCredentials(int id)
{
    provider->removeKey("wifi." + String(id));
}

void Debugger::setupDebug()
{
    _modules.push_back(new DhtMock());
    _modules.push_back(new FanMock());
    _modules.push_back(new FeederMock());
    _modules.push_back(new HeaterMock());
    _modules.push_back(new HumidifierMock());
    _modules.push_back(new LedMock());
    _modules.push_back(new PhProbeMock());
    _modules.push_back(new WaterLevelMock());
    _modules.push_back(new WaterPumpMock());
    _modules.push_back(new WaterTemperatureMock());

    if (debugAddFunctionVoid("Restart", &restart) >= 0)
    {
        debugSetLastFunctionDescription("Restart ESP");
    }

    if (debugAddFunctionVoid("Factory reset", &factoryReset) >= 0)
    {
        debugSetLastFunctionDescription("Factory Reset");
    }

    if (debugAddFunctionVoid("Test memory", &testMemory) >= 0)
    {
        debugSetLastFunctionDescription("Test memory");
    }

    if (debugAddFunctionVoid("Get time", &getTime) >= 0)
    {
        debugSetLastFunctionDescription("Get current time");
    }

    if (debugAddFunctionVoid("Print state", &printState) >= 0)
    {
        debugSetLastFunctionDescription("Print current state");
    }

    if (debugAddFunctionStr("Set temp", &setTemperature) >= 0)
    {
        debugSetLastFunctionDescription("Set temperature");
    }

    if (debugAddFunctionStr("Set loop delay", &setLoopDelay) >= 0)
    {
        debugSetLastFunctionDescription("Set loop delay");
    }

    if (debugAddFunctionVoid("Print heater", &printHeater) >= 0)
    {
        debugSetLastFunctionDescription("Print heater");
    }

    if (debugAddFunctionVoid("Print Triggers", &printTriggers) >= 0)
    {
        debugSetLastFunctionDescription("Print Triggers");
    }

    if (debugAddFunctionVoid("Print FW Version", &printFirmwareVersion) >= 0)
    {
        debugSetLastFunctionDescription("Print FW version");
    }

    if (debugAddFunctionVoid("Switch mock display", &switchMockDisplay) >= 0)
    {
        debugSetLastFunctionDescription("Swtich Mock display..");
    }

    if (debugAddFunctionVoid("Print WiFI credentials", &printCredentials) >= 0)
    {
        debugSetLastFunctionDescription("Print WiFI credentials..");
    }

    debugAddFunctionStr("Setup WiFi pass", &setWiFiPass);
    debugAddFunctionStr("Setup WiFi ssid", &setWiFiSsid);
    debugAddFunctionStr("Setup User ID", &setUserId);
    debugAddFunctionInt("Switch bluetooth, 1 = ON", &switchBluetoth);
    debugAddFunctionInt("Remove credentials (id)", &removeCredentials);
}

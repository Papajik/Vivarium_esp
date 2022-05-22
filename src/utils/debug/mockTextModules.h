#include "../../modules/internal/lcd_display/textModule.h"

class DhtMock : public TextModule
{
    std::vector<String> getText() { return {"DHT:", "T: 19.55 H: 75.00"}; };
};

class FanMock : public TextModule
{
    std::vector<String> getText() { return {"Fan: 90 %", "S: 18 M: 20"}; };
};

class FeederMock : public TextModule
{
    std::vector<String> getText() { return {"Feeder", "Next: 20:´00"}; };
};

class HeaterMock : public TextModule
{
    std::vector<String> getText(){return {"Heater: PID", "G:22.3, PWR:77.5"};};
};

class HumidifierMock : public TextModule
{
    std::vector<String> getText(){return {"Humidifier: ON", "Goal: 70 %"};};
};

class LedMock : public TextModule
{
    std::vector<String> getText() { return {"LED", "No trigger"}; };
};

class PhProbeMock : public TextModule
{
    std::vector<String> getText() { return {"pH Probe", "pH: 6.7"}; };
};

class WaterLevelMock : public TextModule
{
    std::vector<String> getText() { return {"Water Level: 10", "LL: 8 HL: 12"}; };
};

class WaterPumpMock : public TextModule
{
    std::vector<String> getText() { return {
        "Water Pump: OFF",
        "Goal: 15.5 cm"}; };
};

class WaterTemperatureMock : public TextModule
{
    std::vector<String> getText() { return {"Water Temp: 20.1", "LL:19.0 HL:21.0"}; };
};
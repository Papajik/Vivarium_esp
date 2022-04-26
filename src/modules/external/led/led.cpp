#include "led.h"

#define CONNECTED_KEY "led/c"
#define FIREBASE_COLOR_STATE "/led/color"

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include "../../../wifi/wifiProvider.h"
#include <Freenove_WS2812_Lib_for_ESP32.h>

#include "../../../utils/timeHelper.h"
#include "../../../utils/rtc/rtc.h"
#include <string>

void ledTriggerCallback()
{
    AlarmId id = Alarm.getTriggeredAlarmId();
    if (ledModulePtr != nullptr)
    {
        uint32_t color;
        if (ledModulePtr->getTriggerPayload(id, &color))
        {
            printD("Color: ");
            printlnD(String(color));
            ledModulePtr->setColor(color);
        }
    }
    else
    {
        printlnE("Led Module is nullptr");
    }
}

LedModule *ledModulePtr = nullptr;

LedModule::LedModule(int position, MemoryProvider *provider, int pin) : IModule(CONNECTED_KEY, position, provider),
                                                                        PayloadAlarm<uint32_t>(&ledTriggerCallback, provider, "led.")
{
    _strip = new Freenove_ESP32_WS2812(LED_COUNT, pin, LED_CHANNEL, TYPE_GRB);
    _stripConnected = _strip->begin();
    printlnA("Led module created");
}

LedModule::~LedModule()
{
    delete _strip;
    rmt_driver_uninstall((rmt_channel_t)LED_CHANNEL);
}

void LedModule::setColor(uint32_t color)
{
    if (_currentColor != color)
    {
        _currentColor = color;
        _settingsChanged = true;
    }
}

uint32_t LedModule::getColor()
{
    return _currentColor;
}

/// Module
void LedModule::onLoop()
{
    checkConnectionChange();

    if (isConnected())
    {
        showColor();
    }

    if (_settingsChanged)
    {
        saveSettings();
    }
}

void LedModule::showColor(bool force, bool triggered)
{
    if (_currentColor != _lastColor || force)
    {
        _lastColor = _currentColor;
        printI("LED - new color : ");
        printlnI(String(_currentColor));
        _strip->setAllLedsColor(_currentColor);

        uploadColorChange();

        if (isBluetoothRunning())
        {
            _currentColorCharacteristic->setValue(_currentColor);
            _currentColorCharacteristic->notify();
        }
        if (firebaseService != nullptr)
            firebaseService->uploadState(FIREBASE_COLOR_STATE, (int)_currentColor);

        if (messagingService != nullptr)
            messagingService->sendFCM("LED", triggered ? "LED triggered! Color = #" : "Current color=#" + String(_currentColor, 16), FCM_TYPE::TRIGGER, SETTINGS_LED_KEY);
    }
}

void LedModule::uploadColorChange()
{
    printlnV("Upload color change");
    time_t now;
    time(&now);
    char time[40];
    ultoa(now, time, 10);
    if (firebaseService != nullptr)
        firebaseService->uploadCustomData("led/", "/" + std::string(time), std::string(String(_currentColor).c_str()));
}

void LedModule::saveSettings()
{
    _memoryProvider->saveInt(SETTINGS_LED_KEY, _currentColor);
    _settingsChanged = false;
}

bool LedModule::loadSettings()
{
    _currentColor = _memoryProvider->loadInt(SETTINGS_LED_KEY, ((((uint32_t)255 << 0) | ((uint32_t)255 << 8) | ((uint32_t)255 << 16))));

    showColor(false, false);

    if (wifiProvider->isConnected() || rtc.isRunning())
    {
        loadTriggersFromNVS();
        printTriggers();
    }
    return true;
}

void LedModule::onConnectionChange()
{
    sendConnectionChangeNotification("LED", isConnected());
    if (isConnected())
    {
        // Reinit color on new connection + upload data to RTDB
        showColor(true, false);
        uploadColorChange();
    }

    if (_sourceIsButton)
    {
        if (firebaseService != nullptr)
            firebaseService->uploadState(FIREBASE_LED_CONNECTED_KEY, isConnected());
        _sourceIsButton = false;
    }

    if (isBluetoothRunning())
    {
        std::string s = isConnected() ? "true" : "false";
        _connectedCharacteristic->setValue(s);
        _connectedCharacteristic->notify();
    }
}

bool LedModule::isStripConnected()
{
    return _stripConnected;
}

std::vector<String> LedModule::getText()
{
    if (!_connected)
    {
        return {"LED", "Disconnected"};
    }
    else
    {
        int time;
        if (getNextTriggerTime(&time))
        {
            return {"LED", "Next: " + formatTime(time)};
        }
        else
        {
            return {"LED", "No trigger"};
        }
    }
}
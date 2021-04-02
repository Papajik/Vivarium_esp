#include "lcd.h"
#include "aqua_screens.h"

#include <millisDelay.h>
#include <LiquidCrystal_I2C.h>

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

LcdDisplay::LcdDisplay()
{
    _screens.reserve(6);
    _lcd = new LiquidCrystal_I2C(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
}

void LcdDisplay::begin()
{
    _lcd->init();
    _lcd->backlight();
    _delay = new millisDelay();

    _delay->start(lcdSettings.switch_screen_interval);
}

void LcdDisplay::showDefaultScreen()
{
    printlnA("Default screen show");
    _lcd->clear();
    _lcd->setCursor(0, 0);
    _lcd->print(lcdSettings.display_name);
}

void LcdDisplay::tryToRefreshScreen()
{
    if (_delay->justFinished() && !_pin_displayed)
    {
        refreshScreen();
        _delay->repeat();
    }
}

void LcdDisplay::refreshScreen()
{
    if (_screen_count == 0)
    {
        showDefaultScreen();
    }
    else
    {
        nextScreen();
    }
}

void LcdDisplay::setRefreshInterval(int interval)
{
    lcdSettings.switch_screen_interval = interval;
    if (_delay->isRunning())
    {
        _delay->stop();
    }
    _delay->start(interval);
}

void LcdDisplay::nextScreen()
{
    printlnD("Switching screen");
    _current_screen++;
    if (_current_screen == _screen_count)
        _current_screen = 0;
    printV("Switch to ");
    printlnV(_current_screen);
    _screens[_current_screen](_lcd);
}

int LcdDisplay::addScreen(ScreenCallback callback)
{
    _screens.push_back(callback);
    _screen_count++;
    return _screen_count;
}

int LcdDisplay::addScreen(ScreenCallback callback, int position)
{
    auto pos = _screens.begin() + position;
    _screens.insert(pos, callback);
    _screen_count++;
    return _screen_count;
}

void LcdDisplay::showPIN(int pin)
{
    printD("Showing PIN on display = ");
    printlnD(pin);
    _pin_displayed = true;
    _lcd->clear();
    _lcd->setCursor(0, 0);
    _lcd->print("PIN");
    _lcd->setCursor(0, 1);
    _lcd->print(pin);
}

void LcdDisplay::hidePIN()
{
    printlnD("Hiding pin, interval = ");
    _pin_displayed = false;
    printlnD(lcdSettings.switch_screen_interval);

    // to prevent multiple refresh screen.
    //_delay->repeat() in refreshScreen() would be called multiple times to catch up with current time otherwise
    _delay->restart();
    refreshScreen();
}

LcdDisplay lcdDisplay;
LcdSettings lcdSettings = {DEFAULT_LCD_INTERVAL, DEFAULT_LCD_NAME};
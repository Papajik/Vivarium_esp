#include "lcd.h"
#include "aqua_screens.h"
#include <stdio.h>
#include <millisDelay.h>
#include <LiquidCrystal_I2C.h>

#include <HardwareSerial.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug

#include "../../../state/state.h"

LcdDisplay::LcdDisplay()
    : _delay(new millisDelay()),
      _lcd(new LiquidCrystal_I2C(LCD_ADDRESS, LCD_COLS, LCD_ROWS))
{
    // _screens.reserve(6);
    _modules.reserve(8);
}
LcdDisplay::~LcdDisplay()
{
    delete _lcd;
    delete _delay;
}

void LcdDisplay::begin()
{
    _lcd->init();
    _lcd->backlight();
    showDefaultScreen();

    _delay->start(lcdSettings.switch_screen_interval);
}

void LcdDisplay::showDefaultScreen()
{
    printlnD("Default screen show");
    _lcd->clear();
    _lcd->setCursor(0, 0);
    _lcd->print(lcdSettings.display_name);
}

void LcdDisplay::resetTimer()
{
    if (_delay->isRunning())
    {
        _delay->restart();
    }
}

void LcdDisplay::onTextUnlocked()
{
    resetTimer();
}

void LcdDisplay::onLoop()
{
    //Check if text is ready to be unlocked
    checkLockedTextDelay();

    // Pin has highest priory
    if (_to_display)
    {
        showPIN();
    }

    if (_to_hide)
    {
        _delay->restart();
        if (!_textLocked)
        {
            refreshScreen();
        }
    }

    // Second priority is textOutput
    if (_textLocked)
    {
        return;
    }

    if (_delay->justFinished() && !_is_pin_displayed)
    {
        refreshScreen();

        _delay->repeat();
        // if there is problem with time change
        if (_delay->remaining() == 0)
        {
            _delay->restart();
        }
    }
}

void LcdDisplay::refreshScreen()
{
    if (_modules.empty())
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
    printA("Switching screen:");
    _current_screen++;
    printlnD(_current_screen);

    if (_current_screen < _modules.size())
    {
        displayTextFromVector(_modules[_current_screen]->getText());
    }

    if (_current_screen == _modules.size())
    {
        showUpdateStatus();
    }

    if (_current_screen == _modules.size() + 1)
        _current_screen = 0;
}

void LcdDisplay::showUpdateStatus()
{
    printlnD("displayUpdateStatus");
    _lcd->clear();
    _lcd->setCursor(0, 0);
    _lcd->print("Last update ");

    _lcd->setCursor(0, 1);

    char LCDTime[] = "000000000000000";

    time_t t = stateStorage.getLastUpdate();

    if (t != 0)
    {
        tm *time = localtime(&t);

        sprintf(LCDTime, "%02d:%02d:%02d %02d.%02d.", time->tm_hour, time->tm_min, time->tm_sec, time->tm_mday, time->tm_mon);
        _lcd->print(LCDTime);
    }
    else
    {
        _lcd->print("--:--");
    }
}

void LcdDisplay::displayTextFromVector(std::vector<String> texts)
{
    _lcd->clear();
    if (!texts.empty())
    {
        _lcd->setCursor(0, 0);
        printlnD(texts[0]);
        _lcd->print(texts[0]);
        if (texts.size() > 1)
        {
            printlnD(texts[1]);
            _lcd->setCursor(0, 1);
            _lcd->print(texts[1]);
        }
    }
    else
    {
        showDefaultScreen();
    }
}

// int LcdDisplay::addScreen(ScreenCallback callback)
// {
//     _screens.push_back(callback);
//     _screen_count++;
//     return _screen_count;
// }

// int LcdDisplay::addScreen(ScreenCallback callback, int position)
// {
//     auto pos = _screens.begin() + position;
//     _screens.insert(pos, callback);
//     _screen_count++;
//     return _screen_count;
// }

int LcdDisplay::addModule(TextModule *module)
{
    _modules.push_back(module);
    return _modules.size();
}

void LcdDisplay::showPIN()
{
    _to_display = false;
    printA("Showing PIN on display = ");
    printlnA(_pin);
    _is_pin_displayed = true;
    _lcd->clear();
    _lcd->setCursor(0, 0);
    _lcd->print("PIN");
    _lcd->setCursor(0, 1);
    _lcd->print(_pin);
}

void LcdDisplay::hidePIN()
{
    printlnD("Hiding pin");
    _to_hide = true;
    _is_pin_displayed = false;
}

void LcdDisplay::setPINToShow(int pin)
{
    _pin = pin;
    _to_display = true;
}

void LcdDisplay::displayText(const std::vector<String> &text)
{
    printlnA("Display Text");
    if (text.empty())
        return;
    printlnA("First row");
    printlnA(text[0]);
    _lcd->clear();
    _lcd->setCursor(0, 0);
    _lcd->print(text[0]);

    if (text.size() >= 2)
    {
        printlnA("Second row");
        printlnA(text[1]);
        _lcd->setCursor(0, 1);
        _lcd->print(text[1]);
    }
}

LcdDisplay lcdDisplay;
LcdSettings lcdSettings = {DEFAULT_LCD_INTERVAL, DEFAULT_LCD_NAME};
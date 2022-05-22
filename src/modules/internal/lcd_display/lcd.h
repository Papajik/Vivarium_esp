#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>

#include <vector>
#include "../../../bluetooth/i_bluetooth_pin.h"
#include "textOutput.h"
#include "textModule.h"

// editable settings
#define DEFAULT_LCD_INTERVAL 2500 // 3.5 seconds between screens
#define DEFAULT_LCD_NAME "Smart Vivarium"
#define LCD_NAME_LENGTH 15

//
#define LCD_ADDRESS 0x27
#define LCD_ROWS 2
#define LCD_COLS 16

class LiquidCrystal_I2C;
class millisDelay;

typedef void (*ScreenCallback)(LiquidCrystal_I2C *lcd);

class LcdDisplay : public IBluetoothPIN, public TextOutput
{
public:
    LcdDisplay();
    ~LcdDisplay();
    void begin();
    void setRefreshInterval(int interval);
    void onLoop();

    int addModule(TextModule *module);

    void showPIN();
    void hidePIN();
    void setPINToShow(int);

    void resetTimer();
    virtual void onTextUnlocked();
    virtual void displayText(const std::vector<String> &);

private:
    void refreshScreen();
    void nextScreen();
    void showDefaultScreen();
    void showUpdateStatus();

    void displayTextFromVector(std::vector<String>);

    std::vector<TextModule *> _modules;
    int _current_screen = 0;
    millisDelay *_delay;
    LiquidCrystal_I2C *_lcd;
};

struct LcdSettings
{
    uint32_t switch_screen_interval;
    char display_name[LCD_NAME_LENGTH];
};

extern LcdSettings lcdSettings;
extern LcdDisplay lcdDisplay;

#endif
#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>

#include <vector>
#include "../../../bluetooth/i_bluetooth_pin.h"

// editable settings
#define DEFAULT_LCD_INTERVAL 5000 // 5 seconds between screens
#define DEFAULT_LCD_NAME "Vivarium"
#define LCD_NAME_LENGTH 10

//
#define LCD_ADDRESS 0x27
#define LCD_ROWS 2
#define LCD_COLS 16

class LiquidCrystal_I2C;
class millisDelay;

typedef void (*ScreenCallback)(LiquidCrystal_I2C *lcd);

class LcdDisplay : public IBluetoothPIN
{
public:
    LcdDisplay();
    void begin();
    void setRefreshInterval(int interval);
    void onLoop();
    int addScreen(ScreenCallback callback);
    int addScreen(ScreenCallback callback, int position);
    void showPIN();
    void hidePIN();
    void setPINToShow(int);

private:
    void refreshScreen();
    void nextScreen();
    void showDefaultScreen();

    std::vector<ScreenCallback> _screens;
    int _screen_count = 0;
    int _current_screen = 0;
    

    LiquidCrystal_I2C *_lcd;
    millisDelay *_delay;
};

struct LcdSettings
{
    uint32_t switch_screen_interval;
    char display_name[LCD_NAME_LENGTH];
};

extern LcdSettings lcdSettings;
extern LcdDisplay lcdDisplay;

#endif
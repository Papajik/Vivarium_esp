#include "aqua_screens.h"

#include <time.h>
#include "lcd.h"

#include "../../../state/state.h"
#include "../../../state/state_values.h"
#include "../../../settings/settings.h"

#include "../../external/heater/heater.h"

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <HardwareSerial.h>

void displayTemperature(LiquidCrystal_I2C *lcd)
{
    printlnD("displayTemperature");
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("T1: ");

    float t;
    int response = stateStorage.getValue(STATE_WATER_TEMPERATURE, &t);
    float goal;
    ;
    if (response != -1)
    {
        lcd->print(t, 2);
    }
    else
    {
        lcd->print("NaN");
    }

    lcd->print((char)223);
    lcd->print("C");
    lcd->setCursor(0, 1);
    lcd->print("Goal: ");
    if (stateStorage.getValue(TEMP_GOAL, &goal))
    {
        lcd->print(goal, 2);
        lcd->print((char)223);
        lcd->print("C");
    }
    else
    {
        lcd->print(" NaN");
    }
}

void displayUpdateStatus(LiquidCrystal_I2C *lcd)
{
    printlnD("displayUpdateStatus");
    return;
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Last update ");

    lcd->setCursor(0, 1);

    char LCDTime[] = "000000000000000";

    time_t t = stateStorage.getLastUpdate();
    tm *time = localtime(&t);

    sprintf(LCDTime, "%02d.%02d. %02d:%02d:%02d", time->tm_year, time->tm_mon, time->tm_hour, time->tm_min, time->tm_sec);
    lcd->print(LCDTime);
}

void displayWaterStatus(LiquidCrystal_I2C *lcd)
{
    printlnD("displayWaterStatus");
    return;
    lcd->clear();
    lcd->setCursor(0, 0);   
    lcd->print("water");
}

void displayPhAndSockets(LiquidCrystal_I2C *lcd)
{
    printlnD("displayPhAndSockets");
    return;
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("ph");
}

void displayHeater(LiquidCrystal_I2C *lcd)
{
    printlnV("Display heater");

    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Fan: ");
    uint32_t s;
    if (stateStorage.getValue(FAN_SPEED, &s))
    {
        lcd->print(s);
        lcd->print(" %");
    }
    else
    {
        lcd->print("NaN");
    }

    lcd->setCursor(0, 1);

    lcd->print("Heater: ");
    float power;
    if (stateStorage.getValue(HEATER_POWER, &power))
    {
        printD("Power = ");
        printlnD(power);
        if (power == GOAL_INVALID)
        {
            printlnD("LCD Screens - Goal is invalid");
            lcd->print("NaN");
        }
        else
        {
            lcd->print(power, 2);
            lcd->print(" %");
        }
    }
    else
    {
        lcd->print("NaN");
    }
}

void setAquariumScreens(LcdDisplay *lcdDisplay)
{
    lcdDisplay->addScreen(displayTemperature);
    lcdDisplay->addScreen(displayHeater);
    // lcdDisplay->addScreen(displayTimers);
    // lcdDisplay->addScreen(displayUpdateStatus);
    // lcdDisplay->addScreen(displayWaterStatus);
    // lcdDisplay->addScreen(displayPhAndSockets);
}
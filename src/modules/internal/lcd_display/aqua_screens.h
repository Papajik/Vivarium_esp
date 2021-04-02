#include <LiquidCrystal_I2C.h>


class LiquidCrystal_I2C;
class LcdDisplay;

void displayTemperature(LiquidCrystal_I2C *lcd);

void displayUpdateStatus(LiquidCrystal_I2C *lcd);

void displayWaterStatus(LiquidCrystal_I2C *lcd);

void displayPhAndSockets(LiquidCrystal_I2C *lcd);

void displayHeater(LiquidCrystal_I2C *lcd);

void setAquariumScreens(LcdDisplay *lcdDisplay);
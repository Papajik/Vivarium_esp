/**
* @file analog.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-06
* 
* @copyright Copyright (c) 2021
* 
*/

#ifndef _V_ANALOG_H_
#define _V_ANALOG_H_
#include <math.h>
#include <esp32-hal-adc.h>

#define ADC_RESOLUTION 2047 //  11 bit

/**
* @brief Setup analog resolution
* 
*/
void initAnalog();


#endif
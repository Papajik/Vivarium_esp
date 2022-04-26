/**
* @file moduleType.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/

#ifndef MODULE_TYPE_H
#define MODULE_TYPE_H

/**
* @brief Definition of all supported module types 
* 
*/
enum ModuleType
{
    DHT_MODULE,
    FAN,
    FEEDER,
    HEATER,
    HUMIDIFIER,
    LED,
    PH_PROBE,
    WATER_LEVEL,
    WATER_PUMP,
    WATER_TEMPERATURE
};

#endif
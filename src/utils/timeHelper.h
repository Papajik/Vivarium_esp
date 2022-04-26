/**
* @file timeHelper.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-09-21
* 
* @copyright Copyright (c) 2021
* 
*/

#ifndef _TIME_HELPER_H
#define _TIME_HELPER_H

#include <WString.h>

/**
* @brief Get time of day from hour and minute
* 
* @param hour 
* @param minute 
* @return int 
*/
int getTime(int hour, int minute);

/**
* @brief Format time into HH:MM String from Hour and Minute
* 
* @param hour 
* @param minute 
* @return String 
*/
String formatTime(int hour, int minute);

/**
* @brief Format time into HH:MM String from time of day
* 
* @param time 
* @return String 
*/
String formatTime(int time);

#endif

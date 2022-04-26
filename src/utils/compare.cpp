/**
* @file compare.cpp
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-09-20
* 
* @copyright Copyright (c) 2021
* 
*/

#include "compare.h"

int compareFloat(const void *arg1, const void *arg2)
{
    float *a = (float *)arg1;
    float *b = (float *)arg2;
    if (*a < *b)
        return -1;
    if (*a > *b)
        return 1;
    return 0;
}

int compareInt(const void *arg1, const void *arg2)
{
    int *a = (int *)arg1;
    int *b = (int *)arg2;
    if (*a < *b)
        return -1;
    if (*a > *b)
        return 1;
    return 0;
}
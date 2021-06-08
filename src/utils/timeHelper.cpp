#include "timeHelper.h"
#include <stdio.h>

int getTime(int hour, int minute)
{
    return hour * 256 + minute;
}

String formatTime(int hour, int minute)
{
    char buffer[50];
    sprintf(buffer, "%02d:%02d", hour, minute);
    return String(buffer);
}

String formatTime(int time)
{
    return formatTime(time / 256, time % 256);
}
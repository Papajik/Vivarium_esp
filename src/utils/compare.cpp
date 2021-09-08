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
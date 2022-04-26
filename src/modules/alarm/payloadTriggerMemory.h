#ifndef PAYLOAD_TRIGGER_MEMORY_H
#define PAYLOAD_TRIGGER_MEMORY_H

template <typename T>
struct PayloadTriggerMemory
{
    int hour;
    int minute;
    T payload;
    char key[25];
};

#endif
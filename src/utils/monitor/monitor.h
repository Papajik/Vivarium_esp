/**
* @file monitor.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-09-20
* 
* @copyright Copyright (c) 2021
* 
*/

#ifndef _MONITOR_H
#define _MONITOR_H

typedef void (*callback)(void);
class Vivarium;

class Monitor
{
public:
    Monitor();
    ~Monitor();
    callback getCallback();
    void print();

    void addVivarium(Vivarium *);

private:
    Vivarium *_vivarium = nullptr;
};

extern Monitor monitor;

#endif
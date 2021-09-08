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
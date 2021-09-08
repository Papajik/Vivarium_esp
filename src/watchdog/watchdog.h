#ifndef V_WATCHDOG_H_
#define V_WATCHDOG_H_

class Vivarium;
typedef void (*callback)(void);

class Watchdog
{

public:
    Watchdog();
    void addVivarium(Vivarium *);
    callback getCallback();
    void checkDeadlock();

private:
    Vivarium *_vivarium;
    unsigned long lastDelayTime = 0;
};

extern Watchdog watchdog;

#endif
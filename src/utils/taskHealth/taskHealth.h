#ifndef _TASK_HEALTH_H
#define _TASK_HEALTH_H

class TaskHealth
{
public:
    TaskHealth();
    ~TaskHealth();
    bool isAlive();

protected:
    void pingAlive();

private:
    SemaphoreHandle_t mutex;
    unsigned long _run;
    unsigned long _lastAlive;
};

#endif
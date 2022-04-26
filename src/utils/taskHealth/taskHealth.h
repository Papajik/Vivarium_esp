/**
* @file taskHealth.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-09-20
* 
* @copyright Copyright (c) 2021
* 
*/

#ifndef _TASK_HEALTH_H
#define _TASK_HEALTH_H

/**
* @brief Checks health of inheriting class. Used to dermine if onLoop() is being called periodally
* 
*/
class TaskHealth
{
public:
    TaskHealth();
    ~TaskHealth();
    bool isAlive();
    unsigned long getLastAlive();

protected:
    void pingAlive();

private:
    SemaphoreHandle_t mutex;
    unsigned long _run;
    unsigned long _lastAlive;
};

#endif
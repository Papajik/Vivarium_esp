/**
* @file runner.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef _RUNNER_H_
#define _RUNNER_H_

#define MAX_CALLBACKS 3

typedef void (*callback)(void);

/**
* @brief Parallel Runner starts secondary task and runs given callback inside its loop
* 
*   Multiple callbacks can be added. 
*/
class ParallelRunner
{
public:
    ParallelRunner();
    bool addCallback(callback);
    void startRunner();
    callback getNextCallback();

private:
    int _lastCallback = 0;
    int _callbackCount = 0;
    callback _callbacks[MAX_CALLBACKS];
};

extern ParallelRunner runner;

#endif
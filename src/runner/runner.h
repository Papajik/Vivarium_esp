#ifndef _RUNNER_H_
#define _RUNNER_H_

#define MAX_CALLBACKS 3

typedef void (*callback)(void);

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
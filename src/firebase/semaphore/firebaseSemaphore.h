#ifndef FIREBASE_SEMAPHORE_H
#define FIREBASE_SEMAPHORE_H

#include <WString.h>
#include <FreeRTOS.h>
#include <freertos/semphr.h>


class FirebaseSemaphore
{
public:
    FirebaseSemaphore();
    ~FirebaseSemaphore();

    void lockSemaphore(String owner);
    void unlockSemaphore();

private:
    SemaphoreHandle_t bdoMutex;
    String _owner = "";
};

extern FirebaseSemaphore firebaseSemaphore;

#endif
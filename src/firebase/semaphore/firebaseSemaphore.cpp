#include "firebaseSemaphore.h"

FirebaseSemaphore firebaseSemaphore;

#include <SerialDebug.h>

FirebaseSemaphore::FirebaseSemaphore()
{
    bdoMutex = xSemaphoreCreateMutex();
    xSemaphoreGive(bdoMutex);
}

FirebaseSemaphore::~FirebaseSemaphore()
{
    xSemaphoreGive(bdoMutex);
}

void FirebaseSemaphore::lockSemaphore(String owner)
{
    printlnV("Trying to lock semaphore by " + owner);
    xSemaphoreTake(bdoMutex, portMAX_DELAY);
    printlnV("Lock semaphore by " + owner);
    _owner = owner;
}
void FirebaseSemaphore::unlockSemaphore()
{
    printlnV("Unlocking semaphore from " + _owner);
    xSemaphoreGive(bdoMutex);
}

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
    printI("Lock semaphore by " + owner);
    _owner = owner;
    xSemaphoreTake(bdoMutex, portMAX_DELAY);
}
void FirebaseSemaphore::unlockSemaphore()
{
    xSemaphoreGive(bdoMutex);
    printI("Unlock semaphore from " + _owner);
}

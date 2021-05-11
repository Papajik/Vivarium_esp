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
    printI("Lock semaphore by ");
    _owner = owner;
    printlnI(owner);
    xSemaphoreTake(bdoMutex, portMAX_DELAY);
}
void FirebaseSemaphore::unlockSemaphore()
{
    xSemaphoreGive(bdoMutex);
    printI("Unlock semaphore from ");
    printlnI(_owner);
}

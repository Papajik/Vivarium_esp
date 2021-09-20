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
    Serial.println("Trying to lock semaphore by " + owner);
    xSemaphoreTake(bdoMutex, portMAX_DELAY);
    Serial.println("Lock semaphore by " + owner);
    _owner = owner;
}
void FirebaseSemaphore::unlockSemaphore()
{
    Serial.println("Unlocking semaphore from " + _owner);
    xSemaphoreGive(bdoMutex);
}

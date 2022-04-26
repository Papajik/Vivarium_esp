/**
* @file firebaseSemaphore.h
* @author Michal Papaj (papaj.mich@gmail.com)
* @brief 
* @version 1.0
* @date 2021-12-08
* 
* @copyright Copyright (c) 2021
* 
*/
#ifndef FIREBASE_SEMAPHORE_H
#define FIREBASE_SEMAPHORE_H

#include <WString.h>
#include <FreeRTOS.h>
#include <freertos/semphr.h>

/**
* @brief Semaphore for asynchronous access to Firebase services
* 
*/
class FirebaseSemaphore
{
public:
    FirebaseSemaphore();
    ~FirebaseSemaphore();

    /**
    * @brief Tries to lock the FreeRTOS semaphore
    * 
    * @param owner name of the element invoking this method
    */
    void lockSemaphore(String owner);

    void unlockSemaphore();

private:
    SemaphoreHandle_t bdoMutex;
    String _owner = "";
};

extern FirebaseSemaphore firebaseSemaphore;

#endif
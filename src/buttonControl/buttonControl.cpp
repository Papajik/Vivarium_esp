#include "buttonControl.h"
#include "../analog/analog.h"

#include "../bluetooth/bluetooth.h"
#include "../moduleControl/moduleControl.h"
#include "../firebase/firebase.h"

#include <freertos/task.h>

#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <WString.h>

ButtonControl *buttonControl;

#define DEBOUNCE_INTERVAL 200

void readAnalogTask(void *parameter)
{
    printlnA("readAnalogTask");

    int buttonCount = MODULE_COUNT + 1;
    float avg = ADC_RESOLUTION / float(buttonCount);
    int val;
    int n;
    int lastButton = BUTTON_RELEASED;

    for (;;)
    {
        val = analogRead(M_BUTTONS_PIN); // ADC1 - channel 6 on GPIO2
        for (n = 1; n < 16; n++)
            val += analogRead(M_BUTTONS_PIN);
        val /= 16;

        if (val > (buttonCount - 0.5) * avg)
        {
            if (lastButton == BUTTON_RELEASED)
            {
                buttonControl->buttonPressed(BUTTON_RELEASED);
            }
            else
            {
                lastButton = BUTTON_RELEASED;
            }
        }
        else
        {
            for (int i = 0; i < buttonCount; i++)
            {

                if (val < round((i + 0.5) * avg))
                {
                    if (lastButton == i)
                    {
                        printA("Val: ");
                        printA(val);
                        printA("\t =>");
                        printlnA(i);
                        buttonControl->buttonPressed(i);
                    }
                    else
                    {
                        lastButton = i;
                    }

                    break;
                }
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

ButtonControl::ButtonControl(FirebaseService *firebaseService, ModuleControl *moduleControl)
{
    _firebaseService = firebaseService;
    _moduleControl = moduleControl;
}

void ButtonControl::buttonPressed(int button)
{
    if (_lastPressedButton == button || millis() < DEBOUNCE_INTERVAL + _lastPressedTime)
    {
        return;
    }

    _lastPressedTime = millis();
    _lastPressedButton = button;

    printA("Pressed button");
    printlnA(button);
    if (button == BLUETOOTH_BUTTON)
    {
        if (bleController->isRunning())
        {
            printlnA("Stopping bluetooth and starting firebse");
            bleController->setStopInFuture();
            _firebaseService->setStartInFuture();
        }
        else
        {
            printlnA("Starting bluetooth and stopping firebase");
            _firebaseService->setStopInFuture();
            bleController->setStartInFuture();
        }
    }
    else
    {
        _moduleControl->buttonPressed(button);
    }
}

void ButtonControl::start()
{
    xTaskCreate(
        readAnalogTask,   /* Task function. */
        "readAnalogTask", /* String with name of task. */
        5000,             /* Stack size in bytes this task takes 2696 bytes */
        NULL,             /* Parameter passed as input of the task */
        1,                /* Priority of the task.  */
        NULL);            /* Task handle. */
}
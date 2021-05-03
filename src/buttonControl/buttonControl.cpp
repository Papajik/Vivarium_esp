#include "buttonControl.h"
#include "../analog/analog.h"
#include "moduleControl/moduleControl.h"
#include "bluetooth/bluetoothControl.h"
#include <freertos/task.h>

ButtonControl buttonControl;

void readAnalogTask(void *parameter)
{
    printlnA("readAnalogTask");

    int buttonCount = MODULE_COUNT + 1;
    float avg = ADC_RESOLUTION / float(buttonCount);
    int val;
    int n;
    for (;;)
    {
        val = analogRead(M_BUTTONS_PIN); // ADC1 - channel 6 on GPIO2
        for (n = 1; n < 8; n++)
            val += analogRead(M_BUTTONS_PIN);
        val /= 8;
        // printA("Value = ");
        // printlnA(val);
        // printA("Button count = "); printA(buttonCount); printA(", avg =");printA(avg);
        // printA(", over = ");printA((buttonCount - 0.5) * avg); printA(", mc = ");printlnA(float(MODULE_COUNT))

        if (val > (buttonCount - 0.5) * avg)
        {        
            moduleControl.buttonPressed(MODULE_BUTTON_RELEASED);
            bluetoothControl.buttonPressed(BLUETOOTH_BUTTON_RELEASED);
        }
        else
        {
            for (int i = 0; i < buttonCount; i++)
            {

                if (val < round((i + 0.5) * avg))
                {
                    printA("Button pressed: ");
                    printA(i);
                    printA(" (");
                    printA(val);
                    printlnA(")");  

                    if (i == BLUETOOTH_BUTTON)
                    {
                        bluetoothControl.buttonPressed(i);
                    }
                    else
                    {
                        moduleControl.buttonPressed(MODULE_COUNT - i - 1);
                    }

                    break;

                    // if (i < MODULE_COUNT)
                    // {
                    //     moduleControl.buttonPressed(i);
                    // }
                    // else

                    // {
                    //     bluetoothControl.buttonPressed(i);
                    // }

                    // break;
                }
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

ButtonControl::ButtonControl()
{
}

void ButtonControl::start()
{
    xTaskCreate(
        readAnalogTask,   /* Task function. */
        "readAnalogTask", /* String with name of task. */
        5000,             /* Stack size in bytes this task takes 2696 bytes */
        NULL,             /* Parameter passed as input of the task */
        5,                /* Priority of the task. */
        NULL);            /* Task handle. */
}
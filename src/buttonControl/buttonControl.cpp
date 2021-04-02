#include "buttonControl.h"
#include "../analog/analog.h"
#include "moduleControl/moduleControl.h"
#include "bluetooth/bluetoothControl.h"
#include <freertos/task.h>

void readAnalogTask(void *parameter)
{

    int buttonCount = MODULE_COUNT + BLUETOOTH_BUTTON;
    float avg = ADC_RESOLUTION / float(MODULE_COUNT);
    int val;
    int n;
    for (;;)
    {
        val = analogRead(M_BUTTONS_PIN); // ADC1 - channel 6 on GPIO2
        for (n = 1; n < 8; n++)
            val += analogRead(M_BUTTONS_PIN);
        val /= 8;

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
                    if (i < MODULE_COUNT)
                    {
                        moduleControl.buttonPressed(i);
                    }
                    else
                    {
                        bluetoothControl.buttonPressed(i);
                    }

                    break;
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
        3000,             /* Stack size in bytes this task takes 2696 bytes */
        NULL,             /* Parameter passed as input of the task */
        5,                /* Priority of the task. */
        NULL);            /* Task handle. */
}
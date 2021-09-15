#include "memory.h"
#include <Esp.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <HardwareSerial.h>
// #include <freertos/task.h>
// #include <esp_heap_task_info.h>

// #define MAX_TASK_NUM 20                         // Max number of per tasks info that it can store
// #define MAX_BLOCK_NUM 20                        // Max number of per block info that it can store

// size_t s_prepopulated_num = 0;
// heap_task_totals_t s_totals_arr[MAX_TASK_NUM];
// heap_task_block_t s_block_arr[MAX_BLOCK_NUM];

void printMemory()
{

    Serial.println("\n*** *** *** *** *** ***");
    // int freestack = uxTaskGetStackHighWaterMark(NULL);
    // uint32_t h = xPortGetFreeHeapSize();
    // printlnA("xPortGetFreeHeapSize = " + String(h));
    // printA("Free stack = ");
    // printlnA(freestack);
    Serial.println("ESP heapSize: " + String(ESP.getHeapSize()));
    Serial.println("ESP freeHeap: " + String(ESP.getFreeHeap()));
    Serial.println("ESP minFreeHeap:" + String(ESP.getMinFreeHeap()));
    Serial.println("ESP maxAllocHeap:" + String(ESP.getMaxAllocHeap()));
    // heap_caps_check_integrity_all(true);
    // debugA("Total PSRAM: %d", ESP.getPsramSize());
    // debugA("Free PSRAM: %d", ESP.getFreePsram());
    Serial.println("*** *** *** *** *** ***\n\n");
}

void printHeapInfo()
{

    printlnA("\n*** *** *** *** *** ***");
    printlnA("Heap info");
    heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);

    printlnA("Check integrity");
    if (heap_caps_check_integrity_all(true))
    {
        printlnA(".. OK");
    }

    printlnA("Largest free block");
    printlnA("Largest free block - " + String(heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT)));
    printlnA("Minimum free size - " + String(heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT)));
    printlnA("\n*** *** *** *** *** ***");
}

// void esp_dump_per_task_heap_info()
// {
//     heap_task_info_params_t heap_info = {0};
//     heap_info.caps[0] = MALLOC_CAP_8BIT;        // Gets heap with CAP_8BIT capabilities
//     heap_info.mask[0] = MALLOC_CAP_8BIT;
//     heap_info.caps[1] = MALLOC_CAP_32BIT;       // Gets heap info with CAP_32BIT capabilities
//     heap_info.mask[1] = MALLOC_CAP_32BIT;
//     heap_info.tasks = NULL;                     // Passing NULL captures heap info for all tasks
//     heap_info.num_tasks = 0;
//     heap_info.totals = s_totals_arr;            // Gets task wise allocation details
//     heap_info.num_totals = &s_prepopulated_num;
//     heap_info.max_totals = MAX_TASK_NUM;        // Maximum length of "s_totals_arr"
//     heap_info.blocks = s_block_arr;             // Gets block wise allocation details. For each block, gets owner task, address and size
//     heap_info.max_blocks = MAX_BLOCK_NUM;       // Maximum length of "s_block_arr"

//     heap_caps_get_per_task_info(&heap_info);

//     for (int i = 0 ; i < *heap_info.num_totals; i++) {
//         printf("Task: %s -> CAP_8BIT: %d CAP_32BIT: %d\n",
//                 heap_info.totals[i].task ? pcTaskGetTaskName(heap_info.totals[i].task) : "Pre-Scheduler allocs" ,
//                 heap_info.totals[i].size[0],    // Heap size with CAP_8BIT capabilities
//                 heap_info.totals[i].size[1]);   // Heap size with CAP32_BIT capabilities
//     }

//     printf("\n\n");
// }
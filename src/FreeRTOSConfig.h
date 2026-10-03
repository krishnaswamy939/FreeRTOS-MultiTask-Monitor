#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*===========================================================
 * BASIC CONFIGURATION
 *==========================================================*/

#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0

#define configCPU_CLOCK_HZ                      1000000
#define configTICK_RATE_HZ                      1000

#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                256

#define configTOTAL_HEAP_SIZE                   (64 * 1024)

#define configMAX_TASK_NAME_LEN                 16


/*===========================================================
 * TASK CONFIGURATION
 *==========================================================*/

#define configUSE_16_BIT_TICKS                  0

#define configIDLE_SHOULD_YIELD                 1


/*===========================================================
 * SYNCHRONIZATION
 *==========================================================*/

#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES            1


/*===========================================================
 * QUEUES
 *==========================================================*/

#define configUSE_QUEUE_SETS                    1


/*===========================================================
 * SOFTWARE TIMERS
 *==========================================================*/

#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               2
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            256


/*===========================================================
 * TASK NOTIFICATIONS
 *==========================================================*/

#define configUSE_TASK_NOTIFICATIONS             1


/*===========================================================
 * MEMORY ALLOCATION
 *==========================================================*/

#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0


/*===========================================================
 * RUN-TIME STATISTICS
 *==========================================================*/

#define configGENERATE_RUN_TIME_STATS           0


/*===========================================================
 * API INCLUSION
 *==========================================================*/

#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskDelayUntil                1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1

#define INCLUDE_xTaskGetSchedulerState          1

#define INCLUDE_uxTaskGetStackHighWaterMark     1

#define INCLUDE_eTaskGetState                   1


/*===========================================================
 * ASSERT
 *==========================================================*/

#define configASSERT(x)                         \
    if ((x) == 0)                               \
    {                                           \
        taskDISABLE_INTERRUPTS();              \
        for (;;)                                \
        {                                       \
        }                                       \
    }


#endif /* FREERTOS_CONFIG_H */
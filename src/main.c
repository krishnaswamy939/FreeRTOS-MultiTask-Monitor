#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"


/*===========================================================
 * CONFIGURATION
 *==========================================================*/

#define SENSOR_QUEUE_LENGTH          5

#define SENSOR_TASK_PRIORITY         3
#define COMM_TASK_PRIORITY           2
#define MONITOR_TASK_PRIORITY        1
#define EVENT_TASK_PRIORITY          2

#define SENSOR_TASK_STACK            1024
#define COMM_TASK_STACK              1024
#define MONITOR_TASK_STACK           1024
#define EVENT_TASK_STACK             1024

#define SENSOR_PERIOD_MS             1000
#define MONITOR_PERIOD_MS            3000
#define TIMER_PERIOD_MS              5000
#define INTERRUPT_PERIOD_MS          700


/*===========================================================
 * GLOBAL RTOS OBJECTS
 *==========================================================*/

/* Queue for sensor data */
static QueueHandle_t sensorQueue;


/* Binary semaphore for event signaling */
static SemaphoreHandle_t eventSemaphore;


/* Mutex for protecting shared printf/logging resource */
static SemaphoreHandle_t printMutex;


/* Software timer */
static TimerHandle_t systemTimer;


/*===========================================================
 * TASK HANDLES
 *==========================================================*/

static TaskHandle_t sensorTaskHandle = NULL;

static TaskHandle_t communicationTaskHandle = NULL;

static TaskHandle_t monitorTaskHandle = NULL;

static TaskHandle_t eventTaskHandle = NULL;


/*===========================================================
 * FUNCTION PROTOTYPES
 *==========================================================*/

static void SensorTask(void *pvParameters);

static void CommunicationTask(void *pvParameters);

static void MonitorTask(void *pvParameters);

static void EventTask(void *pvParameters);

static void SystemTimerCallback(TimerHandle_t xTimer);

static void SimulatedInterrupt(void);


/*===========================================================
 * SAFE PRINT FUNCTION
 *
 * Mutex protects printf() from multiple tasks.
 *==========================================================*/

static void SafePrint(const char *message)
{
    if (xSemaphoreTake(
            printMutex,
            portMAX_DELAY) == pdPASS)
    {
        printf("%s", message);

        xSemaphoreGive(printMutex);
    }
}


/*===========================================================
 * SENSOR TASK
 *
 * Priority = 3
 *
 * Responsibilities:
 *
 * 1. Generate sensor data
 * 2. Send data to Queue
 * 3. Give Binary Semaphore
 * 4. Notify Communication Task
 *==========================================================*/

static void SensorTask(void *pvParameters)
{
    (void) pvParameters;

    int sensorValue = 0;

    while (1)
    {
        sensorValue++;


        /*---------------------------------------------------
         * Generate sensor value
         *--------------------------------------------------*/

        if (xSemaphoreTake(
                printMutex,
                portMAX_DELAY) == pdPASS)
        {
            printf(
                "[Sensor Task] Sensor Value = %d\n",
                sensorValue
            );

            xSemaphoreGive(printMutex);
        }


        /*---------------------------------------------------
         * Send sensor value to Queue
         *--------------------------------------------------*/

        if (xQueueSend(
                sensorQueue,
                &sensorValue,
                portMAX_DELAY) == pdPASS)
        {
            SafePrint(
                "[Sensor Task] Data sent to Queue.\n"
            );
        }


        /*---------------------------------------------------
         * Give Binary Semaphore
         *--------------------------------------------------*/

        if (xSemaphoreGive(eventSemaphore) == pdPASS)
        {
            SafePrint(
                "[Sensor Task] Binary Semaphore given.\n"
            );
        }


        /*---------------------------------------------------
         * Task Notification
         *
         * Notify Communication Task that new data
         * is available.
         *--------------------------------------------------*/

        xTaskNotifyGive(
            communicationTaskHandle
        );

        SafePrint(
            "[Sensor Task] Communication Task notified.\n"
        );


        /*---------------------------------------------------
         * Wait 1 second
         *--------------------------------------------------*/

        vTaskDelay(
            pdMS_TO_TICKS(SENSOR_PERIOD_MS)
        );
    }
}


/*===========================================================
 * COMMUNICATION TASK
 *
 * Priority = 2
 *
 * Responsibilities:
 *
 * 1. Receive sensor data
 * 2. Wait for task notification
 * 3. Process received data
 *==========================================================*/

static void CommunicationTask(void *pvParameters)
{
    (void) pvParameters;

    int receivedValue;


    while (1)
    {
        /*---------------------------------------------------
         * Wait for sensor data
         *--------------------------------------------------*/

        if (xQueueReceive(
                sensorQueue,
                &receivedValue,
                portMAX_DELAY) == pdPASS)
        {
            if (xSemaphoreTake(
                    printMutex,
                    portMAX_DELAY) == pdPASS)
            {
                printf(
                    "[Communication Task] "
                    "Received Sensor Value = %d\n",
                    receivedValue
                );

                xSemaphoreGive(printMutex);
            }
        }


        /*---------------------------------------------------
         * Wait for Task Notification
         *--------------------------------------------------*/

        if (ulTaskNotifyTake(
                pdTRUE,
                portMAX_DELAY) > 0)
        {
            SafePrint(
                "[Communication Task] "
                "Task Notification received.\n"
            );
        }
    }
}


/*===========================================================
 * MONITOR TASK
 *
 * Priority = 1
 *
 * Responsibilities:
 *
 * 1. Monitor task states
 * 2. Monitor stack usage
 * 3. Receive software timer notification
 *==========================================================*/

static void MonitorTask(void *pvParameters)
{
    (void) pvParameters;


    while (1)
    {
        SafePrint(
            "\n-----------------------------------------\n"
        );

        SafePrint(
            "[Monitor Task] System Monitoring\n"
        );

        SafePrint(
            "-----------------------------------------\n"
        );


        /*---------------------------------------------------
         * Sensor Task State
         *--------------------------------------------------*/

        eTaskState sensorState =
            eTaskGetState(sensorTaskHandle);


        /*---------------------------------------------------
         * Communication Task State
         *--------------------------------------------------*/

        eTaskState communicationState =
            eTaskGetState(
                communicationTaskHandle
            );


        /*---------------------------------------------------
         * Event Task State
         *--------------------------------------------------*/

        eTaskState eventState =
            eTaskGetState(eventTaskHandle);


        /*---------------------------------------------------
         * Stack High Water Mark
         *
         * This returns the minimum amount of unused
         * stack that remained for the task.
         *--------------------------------------------------*/

        UBaseType_t sensorStack =
            uxTaskGetStackHighWaterMark(
                sensorTaskHandle
            );

        UBaseType_t communicationStack =
            uxTaskGetStackHighWaterMark(
                communicationTaskHandle
            );

        UBaseType_t eventStack =
            uxTaskGetStackHighWaterMark(
                eventTaskHandle
            );


        UBaseType_t monitorStack =
            uxTaskGetStackHighWaterMark(
                monitorTaskHandle
            );


        /*---------------------------------------------------
         * Display Task Information
         *--------------------------------------------------*/

        if (xSemaphoreTake(
                printMutex,
                portMAX_DELAY) == pdPASS)
        {
            printf(
                "[Monitor] Sensor Task State       = %d\n",
                sensorState
            );

            printf(
                "[Monitor] Communication State     = %d\n",
                communicationState
            );

            printf(
                "[Monitor] Event Task State         = %d\n",
                eventState
            );

            printf(
                "[Monitor] Sensor Stack Free        = %lu\n",
                (unsigned long)sensorStack
            );

            printf(
                "[Monitor] Communication Stack Free= %lu\n",
                (unsigned long)communicationStack
            );

            printf(
                "[Monitor] Event Stack Free         = %lu\n",
                (unsigned long)eventStack
            );

            printf(
                "[Monitor] Monitor Stack Free       = %lu\n",
                (unsigned long)monitorStack
            );

            xSemaphoreGive(printMutex);
        }


        /*---------------------------------------------------
         * Wait before next monitoring cycle
         *--------------------------------------------------*/

        vTaskDelay(
            pdMS_TO_TICKS(MONITOR_PERIOD_MS)
        );
    }
}


/*===========================================================
 * EVENT TASK
 *
 * Priority = 2
 *
 * This task represents an event/interrupt handling task.
 *
 * On Windows we don't have a real MCU interrupt here.
 * Instead, SimulatedInterrupt() sends a notification.
 *==========================================================*/

static void EventTask(void *pvParameters)
{
    (void) pvParameters;


    while (1)
    {
        /*---------------------------------------------------
         * Wait until simulated interrupt occurs
         *--------------------------------------------------*/

        if (ulTaskNotifyTake(
                pdTRUE,
                portMAX_DELAY) > 0)
        {
            SafePrint(
                "\n[Event Task] Interrupt/Event received!\n"
            );

            SafePrint(
                "[Event Task] Processing event...\n"
            );

            vTaskDelay(
                pdMS_TO_TICKS(100)
            );

            SafePrint(
                "[Event Task] Event processing completed.\n"
            );
        }
    }
}


/*===========================================================
 * SOFTWARE TIMER CALLBACK
 *
 * This function executes when the FreeRTOS software timer
 * expires.
 *==========================================================*/

static void SystemTimerCallback(
    TimerHandle_t xTimer)
{
    (void) xTimer;


    SafePrint(
        "\n[TIMER] Software Timer expired.\n"
    );


    SafePrint(
        "[TIMER] Periodic system event generated.\n"
    );
}


/*===========================================================
 * SIMULATED INTERRUPT
 *
 * IMPORTANT:
 *
 * This is NOT a real hardware ISR.
 *
 * It simulates an interrupt/event in our Windows
 * FreeRTOS learning environment.
 *==========================================================*/

static void SimulatedInterrupt(void)
{
    if (eventTaskHandle != NULL)
    {
        xTaskNotifyGive(
            eventTaskHandle
        );
    }
}


/*===========================================================
 * INTERRUPT SIMULATION TASK
 *
 * This task periodically generates a simulated interrupt.
 *==========================================================*/

static void InterruptSimulationTask(
    void *pvParameters)
{
    (void) pvParameters;


    while (1)
    {
        vTaskDelay(
            pdMS_TO_TICKS(INTERRUPT_PERIOD_MS)
        );


        SafePrint(
            "\n[Simulator] Generating simulated interrupt...\n"
        );


        SimulatedInterrupt();
    }
}


/*===========================================================
 * MAIN
 *==========================================================*/

int main(void)
{
    printf("\n");

    printf(
        "====================================================\n"
    );

    printf(
        "        FreeRTOS Multi-Task Monitoring System\n"
    );

    printf(
        "====================================================\n"
    );

    printf(
        " Queue + Semaphore + Mutex + Notification\n"
    );

    printf(
        " Software Timer + Task Monitoring + Stack Monitoring\n"
    );

    printf(
        " Interrupt/Event Simulation\n"
    );

    printf(
        "====================================================\n\n"
    );


    /*=======================================================
     * CREATE QUEUE
     *======================================================*/

    sensorQueue =
        xQueueCreate(
            SENSOR_QUEUE_LENGTH,
            sizeof(int)
        );


    if (sensorQueue == NULL)
    {
        printf(
            "[ERROR] Sensor Queue creation failed!\n"
        );

        return 1;
    }


    printf(
        "[MAIN] Sensor Queue created successfully.\n"
    );


    /*=======================================================
     * CREATE BINARY SEMAPHORE
     *======================================================*/

    eventSemaphore =
        xSemaphoreCreateBinary();


    if (eventSemaphore == NULL)
    {
        printf(
            "[ERROR] Binary Semaphore creation failed!\n"
        );

        return 1;
    }


    printf(
        "[MAIN] Binary Semaphore created successfully.\n"
    );


    /*=======================================================
     * CREATE MUTEX
     *======================================================*/

    printMutex =
        xSemaphoreCreateMutex();


    if (printMutex == NULL)
    {
        printf(
            "[ERROR] Mutex creation failed!\n"
        );

        return 1;
    }


    printf(
        "[MAIN] Mutex created successfully.\n"
    );


    /*=======================================================
     * CREATE SENSOR TASK
     *======================================================*/

    if (xTaskCreate(
            SensorTask,
            "Sensor",
            SENSOR_TASK_STACK,
            NULL,
            SENSOR_TASK_PRIORITY,
            &sensorTaskHandle) != pdPASS)
    {
        printf(
            "[ERROR] Sensor Task creation failed!\n"
        );

        return 1;
    }


    /*=======================================================
     * CREATE COMMUNICATION TASK
     *======================================================*/

    if (xTaskCreate(
            CommunicationTask,
            "Communication",
            COMM_TASK_STACK,
            NULL,
            COMM_TASK_PRIORITY,
            &communicationTaskHandle) != pdPASS)
    {
        printf(
            "[ERROR] Communication Task creation failed!\n"
        );

        return 1;
    }


    /*=======================================================
     * CREATE MONITOR TASK
     *======================================================*/

    if (xTaskCreate(
            MonitorTask,
            "Monitor",
            MONITOR_TASK_STACK,
            NULL,
            MONITOR_TASK_PRIORITY,
            &monitorTaskHandle) != pdPASS)
    {
        printf(
            "[ERROR] Monitor Task creation failed!\n"
        );

        return 1;
    }


    /*=======================================================
     * CREATE EVENT TASK
     *======================================================*/

    if (xTaskCreate(
            EventTask,
            "Event",
            EVENT_TASK_STACK,
            NULL,
            EVENT_TASK_PRIORITY,
            &eventTaskHandle) != pdPASS)
    {
        printf(
            "[ERROR] Event Task creation failed!\n"
        );

        return 1;
    }


    /*=======================================================
     * CREATE INTERRUPT SIMULATION TASK
     *
     * This task generates simulated interrupts.
     *======================================================*/

    if (xTaskCreate(
            InterruptSimulationTask,
            "InterruptSim",
            1024,
            NULL,
            2,
            NULL) != pdPASS)
    {
        printf(
            "[ERROR] Interrupt Simulation Task "
            "creation failed!\n"
        );

        return 1;
    }


    /*=======================================================
     * CREATE SOFTWARE TIMER
     *
     * Timer period = 5 seconds
     *
     * pdTRUE means auto-reload timer.
     *======================================================*/

    systemTimer =
        xTimerCreate(
            "SystemTimer",
            pdMS_TO_TICKS(TIMER_PERIOD_MS),
            pdTRUE,
            NULL,
            SystemTimerCallback
        );


    if (systemTimer == NULL)
    {
        printf(
            "[ERROR] Software Timer creation failed!\n"
        );

        return 1;
    }


    printf(
        "[MAIN] Software Timer created successfully.\n"
    );


    /*=======================================================
     * START SOFTWARE TIMER
     *======================================================*/

    if (xTimerStart(
            systemTimer,
            0) != pdPASS)
    {
        printf(
            "[ERROR] Software Timer start failed!\n"
        );

        return 1;
    }


    printf(
        "[MAIN] Software Timer started.\n"
    );


    /*=======================================================
     * START SCHEDULER
     *======================================================*/

    printf(
        "\n[MAIN] Starting FreeRTOS Scheduler...\n\n"
    );


    vTaskStartScheduler();


    /*=======================================================
     * SHOULD NEVER REACH HERE
     *======================================================*/

    printf(
        "[MAIN] Scheduler stopped unexpectedly.\n"
    );


    return 0;
}
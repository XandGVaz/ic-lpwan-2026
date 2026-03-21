/*
    @author: Vitor Alexandre Garcia Vaz (XandgVaz)
    @date: mar, 2026
    @description: Main file for LoRaWAN node with DHT11 sensor
    This code reads temperature and humidity data from a DHT11 sensor and sends it via LoRaWAN using 
    the MCCI LoRaWAN LMIC library.
*/

/*============================================= Librarys =================================================*/ 
// Arduino core library
#include <Arduino.h>

// FreeRTOS API library
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"

// RTOS manager
#include "task_manager.hpp"
#include "mutex_manager.hpp"
#include "timer_manager.hpp"
#include "queue_event.hpp"

// RTOS Tasks include
#include "task_protocol.hpp"
#include "task_sensor.hpp"


/*================================================ FreeRTOS variables ======================================*/
/*
Task                Core  Prio  Description
-------------------------------------------------------------------------------
vProtocolLoopTask    1     1    Manage LoRaWAN events and joining
vUplinkTask          1     2    Send uplink packets periodically
vSensorTask          0     1    Read DHT sensor data and send to uplink task
-------------------------------------------------------------------------------
*/

// Tasks handles
TaskHandle_t xProtocolLoopTaskHandle = NULL;
TaskHandle_t xUplinkTaskHandle = NULL;
TaskHandle_t xSensorTaskHandle = NULL;

// Timers handles
TimerHandle_t xUplinkTimerHandle = NULL;

// Queues handles and data types
QueueHandle_t xTemperatureQueueHandle = NULL;
QueueHandle_t xHumidityQueueHandle = NULL;

// Mutex handles 
xSemaphoreHandle xDisplatMutex = NULL;

// Task list
TaskList taskList;

/*============================================== Arduino setup and loop ==========================================*/
void setup() {
    // Initialize serial port
    Serial.begin(9600);
    vTaskDelay(pdMS_TO_TICKS(1000));

    /* === Configure RTOS Queues ===*/
    xHumidityQueueHandle = xQueueCreate(3, sizeof(float));
    xTemperatureQueueHandle = xQueueCreate(3, sizeof(float));


    /* === Configure RTOS Timers ===*/
    xUplinkTimerHandle = xTimerCreate(
        "UPLINK_TIMER",
        pdMS_TO_TICKS(UPLINK_DELAY_MS),
        pdTRUE,
        NULL,
        vUplinkTimerCallback
    );

    /* === Configure RTOS Mutex ===*/
    xDisplatMutex = xSemaphoreCreateMutex();

    /* === Configure RTOS Tasks ===*/
    taskList.push_back({
        .handle = xProtocolLoopTaskHandle,
        .function = vProtocolLoopTask,
        .stackSize = configMINIMAL_STACK_SIZE + 1024,
        .parameters = NULL,
        .priority = 1,
        .name = "Protocol"
    });
    taskList.push_back({
        .handle = xSensorTaskHandle,
        .function = vSensorTask,
        .stackSize = configMINIMAL_STACK_SIZE + 1024,
        .parameters = NULL,
        .priority = 2,
        .name = "Sensor"
    });
    createTasks(taskList);
}

void loop() {
    // Delete loop task
    vTaskDelete(NULL);
}
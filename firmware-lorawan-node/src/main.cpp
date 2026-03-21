/*
    @author: Vitor Alexandre Garcia Vaz (XandgVaz)
    @date: jan, 2026
    @description: Main file for LoRaWAN node with DHT22 sensor
    This code reads temperature and humidity data from a DHT22 sensor and sends it via LoRaWAN using 
    the MCCI LoRaWAN LMIC library.
*/

/*============================================= Librarys =================================================*/ 
// Arduino core library
#include <Arduino.h>

// Tasks include
#include "task_protocol.hpp"
#include "task_sensor.hpp"

// FreeRTOS API library
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"

/*================================================ FreeRTOS variables ======================================*/
/*
Task                Core  Prio  Description
-------------------------------------------------------------------------------
vNetworkEventsTask    1     1    Manage LoRaWAN events and joining
vUplinkTask           1     2    Send uplink packets periodically
vDhtTask              0     1    Read DHT sensor data and send to uplink task
-------------------------------------------------------------------------------
*/

// Tasks handles
TaskHandle_t xNetworkEventstaks = NULL;
TaskHandle_t xUplinkTaskHandle = NULL;
TaskHandle_t xDhtTaskHandle = NULL;

// Timers handles
TimerHandle_t xUplinkTimerHandle = NULL;

// Queues handles and data types
QueueHandle_t xTemperatureQueueHandle = NULL;
QueueHandle_t xHumidityQueueHandle = NULL;

// Mutex handles 
xSemaphoreHandle xDisplatMutex = NULL;


/*============================================== Arduino setup and loop ==========================================*/
void setup() {
    // Initialize serial port
    Serial.begin(9600);
    delay(1000);

    /* === Configure RTOS Queues ===*/
    xHumidityQueueHandle = xQueueCreate(3, sizeof(float));
    if(xHumidityQueueHandle == NULL){
        Serial.println("Humidity queue creation failed!");
        while(1);
    }    
    xTemperatureQueueHandle = xQueueCreate(3, sizeof(float));
    if(xTemperatureQueueHandle == NULL){
        Serial.println("Temperature queue creation failed!");
        while(1);
    }

    /* === Configure RTOS Timers ===*/
    xUplinkTimerHandle = xTimerCreate(
        "UPLINK_TIMER",
        pdMS_TO_TICKS(UPLINK_DELAY_MS),
        pdTRUE,
        NULL,
        vUplinkTimerCallback
    );
    if(xUplinkTimerHandle == NULL){
        Serial.println("Uplink timer creation failed!");
        while(1);
    }

    /* === Configure RTOS Mutex ===*/
    xDisplatMutex = xSemaphoreCreateMutex();

    /* === Configure RTOS Tasks ===*/
    xTaskCreatePinnedToCore(
        vNetworkEventsTask,                    
        "NETWORK_EVENTS_TASK",                  
        configMINIMAL_STACK_SIZE + 1024,                 
        NULL,                           
        1,                              
        &xNetworkEventstaks,                 
        APP_CPU_NUM           
    ); 
    if(xNetworkEventstaks == NULL){
        Serial.println("Network events task creation failed!");
        while(1);
    }
    xTaskCreatePinnedToCore(
        vDhtTask,                    
        "DHT_TASK",                  
        configMINIMAL_STACK_SIZE + 1024,                 
        NULL,                           
        1,                              
        &xDhtTaskHandle,                 
        PRO_CPU_NUM           
    );
    if(xDhtTaskHandle == NULL){
        Serial.println("DHT task creation failed!");
        while(1);
    }
}

void loop() {
    // Delete loop task
    vTaskDelete(NULL);
}
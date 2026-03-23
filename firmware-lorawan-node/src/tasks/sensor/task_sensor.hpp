#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Sensor intervalo definitions
#define SENSOR_READ_INTERVAL_MS 20000

// DHT pins
#define DHT_PIN 21

// Tasks handles
extern TaskHandle_t xSensorTaskHandle;

// Tasks prototypes
void vSensorTask(void* pvParameters);
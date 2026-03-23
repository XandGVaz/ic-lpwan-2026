#pragma once
#include "freertos/queue.h"

// Queues handles and data types
extern QueueHandle_t xTemperatureQueueHandle;
extern QueueHandle_t xHumidityQueueHandle;
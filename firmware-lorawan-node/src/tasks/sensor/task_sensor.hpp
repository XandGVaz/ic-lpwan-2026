#ifndef TASK_SENSOR_HPP
#define TASK_SENSOR_HPP

#include "dht.hpp"
#include "queue_event.hpp"

// Sensor intervalo definitions
#define SENSOR_READ_INTERVAL_MS 20000

// DHT pins
#define DHT_PIN 21

// Tasks prototypes
void vDhtTask(void* pvParameters);

#endif /* TASK_SENSOR_HPP */
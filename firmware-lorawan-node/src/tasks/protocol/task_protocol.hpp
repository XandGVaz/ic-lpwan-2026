#ifndef TASK_PROTOCOL_HPP
#define TASK_PROTOCOL_HPP

#include "lorawan.hpp"
#include "display.hpp"
#include "queue_event.hpp"
#include "mutex_manager.hpp"
#include "timer_manager.hpp"
#include "esp_sleep.h"

// Packet definitions 
#define UPLINK_PACKET_SIZE 12
#define UPLINK_DELAY_MS 30000

// Sleep definition
#define PACKETS_BEFORE_SLEEP 1
#define SLEEP_INTERVAL_MS 570000

// LoRa pins
#define LORA_MOSI 27
#define LORA_MISO 19
#define LORA_SCK  5
#define LORA_SS   18

// I2C address and pins
#define OLED_SDA  4    
#define OLED_SCL  15   

// Tasks handles
extern TaskHandle_t xNetworkEventstaks;
extern TaskHandle_t xUplinkTaskHandle;

// Tasks prototypes
void vNetworkEventsTask(void* pvParameters);
void vUplinkTask(void* pvParameters);

#endif /* TASK_PROTOCOL_HPP */
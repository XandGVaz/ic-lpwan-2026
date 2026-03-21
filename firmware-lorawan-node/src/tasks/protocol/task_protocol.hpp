#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
extern TaskHandle_t xProtocolLoopTaskHandle;
extern TaskHandle_t xUplinkTaskHandle;

// Tasks prototypes
void vProtocolLoopTask(void* pvParameters);
void vUplinkTask(void* pvParameters);
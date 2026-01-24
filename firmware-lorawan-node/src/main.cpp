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

// LoRaWANlibrary
#include "lorawan.hpp"

// DHT sensor library
#include "dht.hpp"

// Display library
#include "display.hpp"

// FreeRTOS API library
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"

// Esp32 sleep
#include "esp_sleep.h"

/*============================================== Defines ans consts ===========================================*/ 
// DHT pins
#define DHT_PIN 13

// LoRa pins
#define LORA_MOSI 27
#define LORA_MISO 19
#define LORA_SCK  5
#define LORA_SS   18

// I2C address and pins
#define OLED_SDA  4    
#define OLED_SCL  15   

// LoRa Keys
const uint8_t PROGMEM APPEUI[8] = { 0x28, 0xE4, 0x61, 0xAA, 0xA4, 0xF6, 0xEC, 0xF1 };
const uint8_t PROGMEM DEVEUI[8] = { 0x9D, 0x53, 0x07, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
const uint8_t PROGMEM APPKEY[16] = { 0x12, 0xAF, 0xED, 0xA9, 0x0A, 0x5F, 0xA0, 0x7B, 0xBE, 0x9E, 0x7A, 0xC4, 0x0C, 0x71, 0xDC, 0x95 };

// Packet definitions 
#define UPLINK_PACKET_SIZE 12
#define UPLINK_INTERVAL_MS 180000

// Sleep definition
#define PACKETS_BEFORE_SLEEP 3 
#define SLEEP_INTERVAL_MS  3600000

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

// Tasks prototypes
void vNetworkEventsTask(void* pvParameters);
void vUplinkTask(void* pvParameters);
void vDhtTask(void* pvParameters);

// Timers handles
TimerHandle_t xUplinkTimerHandle = NULL;

// Timers callbacks prototypes
void vUplinkTimerCallback(TimerHandle_t xTimer);

// Queues handles and data types
QueueHandle_t xDhtQueueHandle = NULL;

typedef struct{
    float humidity;
    float temperature;
}dhtQueueData_t;

// Mutex handles 
xSemaphoreHandle xDisplatMutex = NULL;

/*================================================ Global modules ======================================*/
// LoRaWAN
LoRaWAN LoRaWANModule(&SPI);

// DHT module
Dht DHT(DHT_PIN);

// Display module
Display DisplayModule(&Wire);

/*============================================== Arduino setup and loop ==========================================*/
void setup() {
    // Initialize serial port
    Serial.begin(9600);
    delay(1000);

    // Hability light sleep mode
    esp_sleep_enable_timer_wakeup(SLEEP_INTERVAL_MS * 1000ULL); 

    // Display setup
    DisplayModule.setCommunicationPins(OLED_SDA, OLED_SCL);
    if(!DisplayModule.configure()){
        Serial.println("Display setup failed");
        while(1);
    }

    /* Configure RTOS */
    // Create DHT queue
    xDhtQueueHandle = xQueueCreate(3, sizeof(dhtQueueData_t));
    if(xDhtQueueHandle == NULL){
        Serial.println("DHT queue creation failed!");
        while(1);
    }

    // Create and starts Uplink Timer
    xUplinkTimerHandle = xTimerCreate(
        "UPLINK_TIMER",
        pdMS_TO_TICKS(UPLINK_INTERVAL_MS),
        pdTRUE,
        NULL,
        vUplinkTimerCallback
    );
    if(xUplinkTimerHandle == NULL){
        Serial.println("Uplink timer creation failed!");
        while(1);
    }

    // Create Display mutex
    xDisplatMutex = xSemaphoreCreateMutex();

    // Create Network Events Task
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

    // Create DHT Task
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

/*============================================== RTOS callbacks ======================================*/
void vUplinkTimerCallback(TimerHandle_t xTimer){
    // Notify uplink task to send data
    BaseType_t higherPriorityTaskWoken = pdTRUE;
    vTaskNotifyGiveFromISR(xUplinkTaskHandle, &higherPriorityTaskWoken);
    if(higherPriorityTaskWoken = pdTRUE){
        portYIELD_FROM_ISR();
    }
}

/*============================================== RTOS tasks ==========================================*/
void vNetworkEventsTask(void *pvParameters){
    // Configure LoRaWAN module
    LoRaWANModule.setRadioCommunicationPins(LORA_MISO, LORA_MOSI, LORA_SCK, LORA_SS);
    LoRaWANModule.setKeys(APPEUI, DEVEUI, APPKEY);
    LoRaWANModule.configure();

    // Starts node joining in LoRaWAN network
    if(xSemaphoreTake(xDisplatMutex, portMAX_DELAY) == pdTRUE){
        DisplayModule.clear();
        DisplayModule.setCursor(0,SCREEN_HEIGHT/2);
        DisplayModule.println("Joining...", 2);
        xSemaphoreGive(xDisplatMutex);
    }
    LoRaWANModule.iniciateJoin();
    if(xSemaphoreTake(xDisplatMutex, portMAX_DELAY) == pdTRUE){
        DisplayModule.clear();
        DisplayModule.setCursor(0,SCREEN_HEIGHT/2);
        DisplayModule.println("Joined!", 2);
        xSemaphoreGive(xDisplatMutex);
    }

    // Create Uplink Task agter joining started
    xTaskCreatePinnedToCore(
        vUplinkTask,                    
        "UPLINK_TASK",                  
        configMINIMAL_STACK_SIZE + 1024,                 
        NULL,                           
        2,                              
        &xUplinkTaskHandle,                 
        APP_CPU_NUM           
    ); 
    if(xUplinkTaskHandle == NULL){
        Serial.println("Uplink task creation failed!");
        while(1);
    }

    // Process LoRaWAN events
    while(1){
        LoRaWANModule.loop();
    }
}

void vUplinkTask(void* pvParameters){
    
    // DHT data
    dhtQueueData_t dhtQueueData = { .humidity = -100, .temperature = -100 };
    int32_t humidity = -100;
    int32_t temperature = -100;
    
    // RSSI data
    int32_t rssi = 0;

    // Packet
    uint8_t packet[UPLINK_PACKET_SIZE];
    static uint8_t countPacket = 0;

    // Starts Uplink Timer
    if(xTimerStart(xUplinkTimerHandle, 0) == pdFAIL){
        Serial.println("Starts timer failed!");
        while(1);
    }

    while(1){
        // Wait for timer notification
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Get DHT data from queue
        if(xQueueReceive(xDhtQueueHandle, &dhtQueueData, 0) == pdTRUE){

            // Prepare data packet
            humidity = (int32_t)(dhtQueueData.humidity * 100.0);      
            temperature = (int32_t)(dhtQueueData.temperature * 100.0);
            
            // Make packet with raw bytes
            memcpy(packet, &humidity, sizeof(int32_t));
            memcpy(packet + sizeof(int32_t), &temperature, sizeof(int32_t));
        }
        
        // Get last rssi value from LMIC
        rssi = LMIC.rssi;                   
        memcpy(packet + 2 * sizeof(int32_t) , &rssi, sizeof(int32_t));
        
        // Send uplink
        if(xSemaphoreTake(xDisplatMutex, portMAX_DELAY) == pdTRUE){
            DisplayModule.clear();
            DisplayModule.setCursor(0,DisplayModule.height()/2);
            DisplayModule.println("Uplinking...", 2);
            xSemaphoreGive(xDisplatMutex);
        }
        LoRaWANModule.uplink(packet, UPLINK_PACKET_SIZE, 1, false);
        if(xSemaphoreTake(xDisplatMutex, portMAX_DELAY) == pdTRUE){
            DisplayModule.clear();
            DisplayModule.setCursor(0,DisplayModule.height()/2);
            DisplayModule.println("Uplinked!", 2);
            xSemaphoreGive(xDisplatMutex);
        }

        // Increment packet count
        countPacket++;

        // Enter sleep after certain number of packets
        if(countPacket == PACKETS_BEFORE_SLEEP){
            countPacket = 0;
            esp_light_sleep_start();
        }
    }
}

void vDhtTask(void* pvParameters){
    // DHT setup
    if(!DHT.configure()){
        Serial.println("Setup dht sensor failed");
        while(1);
    }

    // DHT data
    dhtQueueData_t dhtQueueData = { .humidity = -1.0, .temperature = -1.0 };

    while(1){
        // Read sensor data
        dhtQueueData.temperature = DHT.getTemperature();
        dhtQueueData.humidity = DHT.getHumidity();

        // Update display
        if(xSemaphoreTake(xDisplatMutex, portMAX_DELAY) == pdTRUE){
            DisplayModule.clear();
            DisplayModule.setCursor(0,0);
            DisplayModule.println("Temp: ", 2);
            DisplayModule.println(String(dhtQueueData.temperature) + " C", 2);
            DisplayModule.println("Hum: ", 2);
            DisplayModule.println(String(dhtQueueData.humidity) + " %", 2);
            xSemaphoreGive(xDisplatMutex);
        }

        // Send data to queue for uplink task
        xQueueSend(xDhtQueueHandle, &dhtQueueData, portMAX_DELAY);

        // Wait for next cycle
        vTaskDelay(pdMS_TO_TICKS(UPLINK_INTERVAL_MS)/30);
    }
}

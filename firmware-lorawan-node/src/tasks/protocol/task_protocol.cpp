#include "task_protocol.hpp"
#include "lorawan.hpp"
#include "display.hpp"
#include "queue_event.hpp"
#include "mutex_manager.hpp"
#include "timer_manager.hpp"
#include "esp_sleep.h"
#include "task_manager.hpp"

// LoRaWAN
LoRaWAN LoRaWANModule(&SPI);

// Display module
Display DisplayModule(&Wire);

// LoRa Keys
const uint8_t PROGMEM APPEUI[8] = { 0x28, 0xE4, 0x61, 0xAA, 0xA4, 0xF6, 0xEC, 0xF1 };
const uint8_t PROGMEM DEVEUI[8] = { 0x9D, 0x53, 0x07, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
const uint8_t PROGMEM APPKEY[16] = { 0x12, 0xAF, 0xED, 0xA9, 0x0A, 0x5F, 0xA0, 0x7B, 0xBE, 0x9E, 0x7A, 0xC4, 0x0C, 0x71, 0xDC, 0x95 };

/*============================================== RTOS callbacks ======================================*/
void vUplinkTimerCallback(TimerHandle_t xTimer)
{
    // Notify uplink task to send data
    BaseType_t higherPriorityTaskWoken = pdTRUE;
    vTaskNotifyGiveFromISR(xUplinkTaskHandle, &higherPriorityTaskWoken);
    if(higherPriorityTaskWoken = pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}

/*============================================== RTOS tasks ==========================================*/
void vProtocolLoopTask(void *pvParameters)
{
    // Hability light sleep mode
    esp_sleep_enable_timer_wakeup(SLEEP_INTERVAL_MS * 1000ULL);
    
    // Display setup
    DisplayModule.setCommunicationPins(OLED_SDA, OLED_SCL);
    if(!DisplayModule.configure())
    {
        if(DEBUG_ON)
            Serial.println("Display setup failed");
        while(1);
    }
    
    // Configure LoRaWAN module
    LoRaWANModule.setRadioCommunicationPins(LORA_MISO, LORA_MOSI, LORA_SCK, LORA_SS);
    LoRaWANModule.setKeys(APPEUI, DEVEUI, APPKEY);
    LoRaWANModule.configure(DR_SF11, 20);

    // Starts node joining in LoRaWAN network
    if(xSemaphoreTake(xDisplatMutex, portMAX_DELAY) == pdTRUE)
    {
        DisplayModule.printCentralText("Joining...", 2);
        xSemaphoreGive(xDisplatMutex);
    }
    LoRaWANModule.iniciateJoin();
    if(xSemaphoreTake(xDisplatMutex, portMAX_DELAY) == pdTRUE)
    {
        DisplayModule.printCentralText("Joined!", 2);
        xSemaphoreGive(xDisplatMutex);
    }

    // Create Uplink Task agter joining started
    TaskStruct uplinkTask = {
        .handle = xUplinkTaskHandle,
        .function = vUplinkTask,
        .stackSize = 4096,
        .parameters = NULL,
        .priority = 2,
        .name = "Uplink"
    };
    createTask(&uplinkTask);

    // Process LoRaWAN events
    while(1)
    {
        LoRaWANModule.loop();
    }
}

void vUplinkTask(void* pvParameters)
{
    // DHT data
    int32_t humidity = -100;
    float humidityFloat = -1.0;
    int32_t temperature = -100;
    float temperatureFloat = -1.0;
    
    // RSSI data
    int32_t rssi = 0;

    // Packet
    uint8_t packet[UPLINK_PACKET_SIZE];
    static uint8_t countPacket = 0;

    // Starts Uplink Timer
    if(xTimerStart(xUplinkTimerHandle, 0) == pdFAIL)
    {
        if(DEBUG_ON)
            Serial.println("Starts timer failed!");
        while(1);
    }

    while(1)
    {
        // Wait for timer notification
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Get DHT data from queue
        if(xQueueReceive(xHumidityQueueHandle, &humidityFloat, 0) == pdTRUE)
        {
            // Convert float to int with 2 decimal places
            humidity = (int32_t)(humidityFloat * 100);

            // Make packet with raw bytes
            memcpy(packet, &humidity, sizeof(int32_t));
        }

        // Get DHT data from queue
        if(xQueueReceive(xTemperatureQueueHandle, &temperatureFloat, 0) == pdTRUE)
        {
            // Convert float to int with 2 decimal places
            temperature = (int32_t)(temperatureFloat * 100); 
            
            // Make packet with raw bytes
            memcpy(packet + sizeof(int32_t), &temperature, sizeof(int32_t));
        }
        
        // Get last rssi value from LMIC
        rssi = LoRaWANModule.getDownlinkRssi();                   
        memcpy(packet + 2 * sizeof(int32_t) , &rssi, sizeof(int32_t));
        
        // Send uplink
        if(xSemaphoreTake(xDisplatMutex, portMAX_DELAY) == pdTRUE)
        {
            DisplayModule.printCentralText("Uplinking...", 2);
            xSemaphoreGive(xDisplatMutex);
        }
        LoRaWANModule.uplink(packet, UPLINK_PACKET_SIZE, 1, false);
        if(xSemaphoreTake(xDisplatMutex, portMAX_DELAY) == pdTRUE)
        {
            DisplayModule.printCentralText("Uplinked!", 2);
            xSemaphoreGive(xDisplatMutex);
        }

        // Update display with sended data
        if(xSemaphoreTake(xDisplatMutex, portMAX_DELAY) == pdTRUE)
        {
            DisplayModule.printTempAndHumidity(temperature, humidity, 2);
            xSemaphoreGive(xDisplatMutex);
        }

        // Increment packet count
        countPacket++;

        // Enter sleep after certain number of packets
        if(countPacket == PACKETS_BEFORE_SLEEP)
        {
            countPacket = 0;
            esp_deep_sleep_start();
        }
    }
}
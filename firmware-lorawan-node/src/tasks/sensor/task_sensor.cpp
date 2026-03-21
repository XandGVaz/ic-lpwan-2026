#include "task_sensor.hpp"
#include "dht.hpp"
#include "queue_event.hpp"

// DHT module
Dht DHT(DHT_PIN);

void vSensorTask(void* pvParameters){    
    // DHT setup
    if(!DHT.configure()){
        Serial.println("Setup dht sensor failed");
        while(1);
    }

    // DHT data
    float humidity = -1.0;
    float temperature = -1.0;

    while(1){
        // Read sensor data
        temperature = DHT.getTemperature();
        humidity = DHT.getHumidity();

        // Send data to queue for uplink task
        xQueueSend(xHumidityQueueHandle, &humidity, portMAX_DELAY);
        xQueueSend(xTemperatureQueueHandle, &temperature, portMAX_DELAY);

        // Wait for next cycle
        vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_INTERVAL_MS));
    }
}
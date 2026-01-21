#ifndef LORAWAN_HPP
#define LORAWAN_HPP

/*============================================= Librarys =================================================*/ 

// MCCI LoRaWAN LMIC library
#include <lmic.h>
#include <hal/hal.h>

// Arduino core libraries
#include <Arduino.h>
#include <SPI.h>

/*=========================================== LoRaWAN class ==============================================*/

// LoRaWAN class
class LoRaWAN {
    SPIClass* _spiHandler = NULL;
    uint8_t _LoRaMiso = 0;
    uint8_t _LoRaMosi = 0;
    uint8_t _LoRaSck = 0;
    uint8_t _LoRaSs = 0;
    osjob_t* _sendjob = NULL;
  public:
    LoRaWAN(SPIClass* spiHandler): _spiHandler(spiHandler) {}
    void setRadioCommunicationPins(uint8_t LoRaMiso, uint8_t LoRaMosi, uint8_t LoRaSck, uint8_t LoRaSs);
    void setKeys(const uint8_t* appEui, const uint8_t* devEui, const uint8_t* appKey);
    void configure();
    void loop();
    void iniciateJoin();
    void uplink(uint8_t* data, uint8_t size, uint8_t port, bool confirmedMode);
};

#endif // LORAWAN_HPP





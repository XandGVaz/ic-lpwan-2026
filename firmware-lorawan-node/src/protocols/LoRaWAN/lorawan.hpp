#ifndef LORAWAN_HPP
#define LORAWAN_HPP

/*============================================= Librarys =================================================*/ 
// MCCI LoRaWAN LMIC library
#include <lmic.h>
#include <hal/hal.h>

// Arduino core libraries
#include <Arduino.h>
#include <SPI.h>

/*============================================= Defines ==================================================*/ 
// RSSI offset for SX1276 LMIC library  
#define LORAWAN_RSSI_OFFSET            64
#define LORAWAN_SX1276_FREQ_LF_MAX     525000000     
#define LORAWAN_SX1272_RSSI_ADJUST     -139
#define LORAWAN_SX1276_RSSI_ADJUST_LF  -164
#define LORAWAN_SX1276_RSSI_ADJUST_HF  -157

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
    void configure(uint8_t spreadFactor = DR_SF7, uint32_t txPower = 14, bool adrMode = false, bool checkLinkMode = false);
    void iniciateJoin();
    void loop();
    void uplink(uint8_t* data, uint8_t size, uint8_t port, bool confirmedMode);
    int32_t getDownlinkSnr();
    int32_t getDownlinkRssi();
};

#endif // LORAWAN_HPP





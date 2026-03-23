#ifndef LORAWAN_HPP
#define LORAWAN_HPP

#include "protocol.h"
#include <Arduino.h>
#include <SPI.h>
#include <lmic.h>
#include <hal/hal.h>

// RSSI offset for SX1276 LMIC library  
#define LORAWAN_RSSI_OFFSET            64
#define LORAWAN_SX1276_FREQ_LF_MAX     525000000     
#define LORAWAN_SX1272_RSSI_ADJUST     -139
#define LORAWAN_SX1276_RSSI_ADJUST_LF  -164
#define LORAWAN_SX1276_RSSI_ADJUST_HF  -157

class LoRaWAN: public Protocol {
  public:
    LoRaWAN(SPIClass* spiHandler): _spiHandler(spiHandler) {}
    void setRadioCommunicationPins(uint8_t LoRaMiso, uint8_t LoRaMosi, uint8_t LoRaSck, uint8_t LoRaSs);
    void setKeys(const uint8_t* appEui, const uint8_t* devEui, const uint8_t* appKey);
    void setConfirmedMode(bool confirmedMode = false);
    void configure(uint16_t packetSize = 128, uint8_t spreadFactor = DR_SF7, uint32_t txPower = 14, bool adrMode = false, bool checkLinkMode = false);
    void iniciateJoin();
    void uplink(const uint8_t* data);
    int32_t getDownlinkRssi();
    int32_t getDownlinkSnr();

    bool connect(const char* server = NULL, int port = 1) override;
    bool isConnected() override;
    void loop();
    void get(const char* resource, Tcallback callback) override;
    void put(const char* resource, const char* payload) override;
  private:
    SPIClass* _spiHandler = NULL;
    uint8_t _LoRaMiso = 0;
    uint8_t _LoRaMosi = 0;
    uint8_t _LoRaSck = 0;
    uint8_t _LoRaSs = 0;
    bool _confirmedMode = false;    
    Tcallback _storedCallback;
};

#endif





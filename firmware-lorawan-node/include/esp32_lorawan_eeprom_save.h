#ifndef ESP32_LORAWAN_EEPROM_SAVE_H
#define ESP32_LORAWAN_EEPROM_SAVE_H

#include "EEPROM.h"

// Classe para gerenciar a EEPROM do ESP32
class Esp32LoRaWANEepromSave{
 private:
    // Ponteiro para lidar com classe EEPROMClass
    EEPROMClass* eeprom = NULL;
 public:
    // Construtor 
    Esp32LoRaWANEepromSave(void):eeprom(new EEPROMClass("LORAWAN_SAVED_DATA")){}

    // Destrutor
    ~Esp32LoRaWANEepromSave(void);

    // Inicializa a EEPROM
    bool begin(void);

    // Grava net ID na EEPROM
    bool writeNetId(uint32_t netId);

    // Grava endereço de dispositivo na EEPROM 
    bool writeDevAddr(uint32_t devAddr);

    // Grava AppSKey na EEPROM
    bool writeAppSKey(const uint8_t* appSKey);

    // Grava NwkSKey na EEPROM
    bool writeNwkSKey(const uint8_t* nwkSKey);

    // Lê net ID da EEPROM
    bool readNetId(uint32_t* netId);

    // Lê endereço de dispositivo da EEPROM
    bool readDevAddr(uint32_t* devAddr);

    // Lê AppSKey da EEPROM
    bool readAppSKey(uint8_t* appSKey);

    // Lê NwkSKey da EEPROM
    bool readNwkSKey(uint8_t* nwkSKey);
};

#endif


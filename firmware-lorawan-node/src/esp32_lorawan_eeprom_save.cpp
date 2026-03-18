#include "esp32_lorawan_eeprom_save.h"

// Destrutor da classe Esp32LoRaWANEepromSave
Esp32LoRaWANEepromSave::~Esp32LoRaWANEepromSave(void){
    // Libera a memória alocada para o ponteiro eeprom
    if(eeprom != NULL) delete eeprom;
    eeprom = NULL;
}

/*Método para inicializar a EEPROM
    parâmetros: nenhum
    retorna:
        - true: inicialização bem-sucedida
        - false: falha na inicialização
*/
bool Esp32LoRaWANEepromSave::begin(void){
    // Verifica se o ponteiro para classe sofreu alocação
    if(eeprom == NULL) return false;
    
    // Tamanho necessário para armazenar netId, devAddr, appSKey e nwkSKey    
    int eepromSize = 
        1 * sizeof(uint32_t) +
        1 * sizeof(uint32_t) +
        16 * sizeof(uint8_t) + 
        16 * sizeof(uint8_t); 

    // Chama o método begin da classe EEPROMClass para inicializar a EEPROM
    return (eeprom->begin(eepromSize));
}

// Método para gravar net ID na EEPROM
bool Esp32LoRaWANEepromSave::writeNetId(uint32_t netId){
    if(eeprom == NULL) return false;
    // Escreve netId na EEPROM usando o método put da classe EEPROMClass
    return (eeprom->put<uint32_t>(0, netId));
}

// Método para gravar endereço de dispositivo na EEPROM
bool Esp32LoRaWANEepromSave::writeDevAddr(uint32_t devAddr){
    if(eeprom == NULL) return false;
    // Escreve devAddr na EEPROM usando o método put da classe EEPROMClass
    return (eeprom->put<uint32_t>(sizeof(uint32_t), devAddr));
}

// Método para gravar AppSKey na EEPROM
bool Esp32LoRaWANEepromSave::writeAppSKey(const uint8_t* appSKey){
    if(eeprom == NULL) return false;
    // Escreve appSKey na EEPROM usando o método writeBytes da classe EEPROMClass
    return (eeprom->writeBytes(
        2 * sizeof(uint32_t), 
        appSKey, 
        16 * sizeof(uint8_t)
    ));
}

bool Esp32LoRaWANEepromSave::writeNwkSKey(const uint8_t* nwkSKey){
    if(eeprom == NULL) return false;
    // Escreve nwkSKey na EEPROM usando o método writeBytes da classe EEPROMClass
    return (eeprom->writeBytes(
        2 * sizeof(uint32_t) + 16 * sizeof(uint8_t),
        nwkSKey,
        16*sizeof(uint8_t)
    ));
}
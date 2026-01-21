#define DEF_MOD_DHT
#include "dht.hpp"

/*===============================================================================*/
// Métodos do  DHT

// Construtor do DHT
Dht::Dht(uint8_t dhtPin){
  this->_dht = new DHT_Unified(dhtPin, DHTTYPE);
  this->_sensor =  new sensor_t;
}

// Método responsável por configurar sensor DHT
bool Dht::setup(){
  // Verifica instanciações de sensor
  if(_dht == NULL || _sensor == NULL) return false;

  // Iniciação do sensor DHT
  _dht->begin();
  _dht->temperature().getSensor(_sensor);
  _dht->humidity().getSensor(_sensor);

  // Retorna sucesso
  return true;
}

// Método que lê informações do sensor e retorna dado de temperatura
float Dht::getTemperature(){
  // Evento de sensoriamento
  sensors_event_t event;  

  // Obtenção da temperatura
  _dht->temperature().getEvent(&event);

  // Verifica erros
  if(isnan(event.temperature)) return -1;
  
  // Obtenção do dado de temperatura  
  return event.temperature;

}

// Método que lê informações do sensor e retorna dado de umidade
float Dht::getHumidity(){
  // Evento de sensoriamento
  sensors_event_t event;      

  // Obtenção da umidade
  _dht->humidity().getEvent(&event);

  // Verifica erros
  if(isnan(event.relative_humidity)) return -1;

  // Obtenção do dado de humidade
  return event.relative_humidity;
}
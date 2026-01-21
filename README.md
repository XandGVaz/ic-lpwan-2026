# ic-lpwan-2026

Projeto de nó LoRaWAN com sensor DHT22 para leitura de temperatura e umidade, utilizando ESP32 Heltec WiFi LoRa 32 V2 e a biblioteca MCCI LoRaWAN LMIC.

## Descrição

Este projeto implementa um nó sensor que lê dados de temperatura e umidade do sensor DHT22 e envia essas informações via LoRaWAN para uma rede LPWAN. O firmware é desenvolvido em C++ usando o framework Arduino e a biblioteca LMIC para comunicação LoRaWAN.

## Estrutura do Projeto

- `firmware-lorawan-node/`
  - `src/`: Código-fonte principal (`main.cpp`, `lorawan.cpp`, `dht.cpp`)
  - `include/`: Headers dos módulos (`lorawan.hpp`, `dht.hpp`)
  - `lib/`: Bibliotecas privadas do projeto
  - `test/`: Testes unitários (PlatformIO)
  - `platformio.ini`: Configuração do PlatformIO

## Principais Funcionalidades

- Leitura de temperatura e umidade via sensor DHT22
- Envio dos dados via LoRaWAN (OTAA) usando ESP32 Heltec
- Estrutura modular para fácil manutenção e expansão

## Dependências

As principais dependências estão listadas no `platformio.ini`:
- MCCI LoRaWAN LMIC library
- Adafruit DHT sensor library
- Adafruit Unified Sensor

## Como usar

1. Instale o [PlatformIO](https://platformio.org/)
2. Clone este repositório
3. Conecte o sensor DHT22 ao pino 13 do ESP32 Heltec
4. Configure as chaves LoRaWAN (AppEUI, DevEUI, AppKey) em `main.cpp` se necessário
5. Compile e faça upload do firmware:
	```
	pio run --target upload
	```
6. Monitore a saída serial:
	```
	pio device monitor
	```

## Licença

MIT
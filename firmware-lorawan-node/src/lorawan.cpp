#include "lorawan.hpp"

/*=========================================== LoRaWAN global variables ==================================*/

// Uplink job
osjob_t Sendjob;
uint8_t DataPacket[12];
uint8_t Port;
uint8_t ConfirmedMode;

// Keys and EUIs
uint8_t APPEUI[8];
uint8_t DEVEUI[8];
uint8_t APPKEY[16];

/*=========================================== LMIC Library functions ======================================*/

// Define keys and EUIs
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

// LoRaWAN pin mapping in esp32 heltec wifi lora 32 V2
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = {26, 35, 34},
};

// Function to print 2-digit hex values with leading zero
void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

// Function to print joining information
void printJoiningInfo() {
    // Session keys
    u4_t netid = 0;
    devaddr_t devaddr = 0;
    u1_t nwkKey[16];
    u1_t artKey[16];

    // Get session keys
    LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);

    // Print session info
    Serial.print("netid: ");
    Serial.println(netid, DEC);
    Serial.print("devaddr: ");
    Serial.println(devaddr, HEX);
    Serial.print("AppSKey: ");
    for (size_t i=0; i<sizeof(artKey); ++i) {
        if (i != 0) Serial.print("-");
        printHex2(artKey[i]);
    }
    Serial.print("\nNwkSKey: ");
    for (size_t i=0; i<sizeof(nwkKey); ++i) {
        if (i != 0) Serial.print("-");
        printHex2(nwkKey[i]);
    }
}

// Function to print transmission infoo
void printTxInfo(){
    // Check for ack and received data
    if (LMIC.txrxFlags & TXRX_ACK){
        Serial.println(F("Received ack"));
    }

    // Check for downlink payload
    if (LMIC.dataLen) {
        Serial.print(F("Received "));
        Serial.print(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
    }
}

// LoRaWAN event handler
void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;

        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;

        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;

        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;

        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;

        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            printJoiningInfo();
            LMIC_setLinkCheckMode(0);
            break;

        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;

        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;

        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            printTxInfo();
            break;

        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;

        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;

        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;

        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;

        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;

        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;

        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;

        case EV_RXSTART:
            break;
            
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            break;

        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

// Function to send LoRaWAN packet
void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(Port, DataPacket, sizeof(DataPacket)-1, ConfirmedMode);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

/*=========================================== LoRaWAN class ==============================================*/

void LoRaWAN::setRadioCommunicationPins(uint8_t LoRaMiso, uint8_t LoRaMosi, uint8_t LoRaSck, uint8_t LoRaSs){
    _LoRaMiso = LoRaMiso;
    _LoRaMosi = LoRaMosi;
    _LoRaSck = LoRaSck;
    _LoRaSs = LoRaSs;
}

void LoRaWAN::setKeys(const uint8_t* appEui, const uint8_t* devEui, const uint8_t* appKey){
    memccpy(APPEUI, appEui, 0, 8);
    memccpy(DEVEUI, devEui, 0, 8);
    memccpy(APPKEY, appKey, 0, 16);
}

void LoRaWAN::configure(){
    // Initialize SPI
    _spiHandler->begin(_LoRaSck, _LoRaMiso, _LoRaMosi, _LoRaSs);

    // Initialize LMIC
    os_init();
    LMIC_reset();

    // Disable all channels of AU915 band
    for (uint8_t i = 0; i < 72; i++) {
        LMIC_disableChannel(i);
    }

    // Habilitate the 8 channels for secondary sub-band (channels 8-15) usging in the AU915 radioenge gateway
    for (uint8_t i = 8; i <= 15; i++) {
        LMIC_enableChannel(i);
    }
    
    // Disable ADR and link check validation, because ADR tends to complicate testing.
    LMIC_setAdrMode(0);
    
    // Disable link check validation, automatically enabled during join
    LMIC_setLinkCheckMode(0);

    // Set static session parameters. Instead of dynamically establishing a session
    LMIC_setDrTxpow(DR_SF12, 14);
}

void LoRaWAN::loop(){
    // Process LMIC events
    os_runloop_once();
}

void LoRaWAN::iniciateJoin(){
    // Start joining procedure
    LMIC_startJoining();
}

void LoRaWAN::uplink(uint8_t* data, uint8_t size, uint8_t port, bool confirmedMode){
    // Copy data to global packet variable
    memcpy(DataPacket, data, size);
    Port = port;
    ConfirmedMode = confirmedMode;

    // Send packet
    do_send(&Sendjob);
}





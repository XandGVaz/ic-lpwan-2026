#include "lorawan.hpp"

/*=========================================== LoRaWAN global variables ==================================*/
// Uplink job
osjob_t Sendjob;
uint16_t PacketSize = 128;
uint8_t *DataPacket = NULL;
uint8_t Port;
uint8_t ConfirmedMode;

// Session keys, Netid and Devaddr, persist in mempry after deep sleep reset
RTC_DATA_ATTR u4_t Netid = 0;
RTC_DATA_ATTR devaddr_t Devaddr = 0;
RTC_DATA_ATTR u1_t NwkKey[16];
RTC_DATA_ATTR u1_t ArtKey[16];

// Variable to verify if the device is already joined after deep sleep reset
RTC_DATA_ATTR bool IsJoined = false;

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
    // Get session keys
    LMIC_getSessionKeys(&Netid, &Devaddr, NwkKey, ArtKey);

    // Print session info
    Serial.print("Netid: ");
    Serial.println(Netid, DEC);
    Serial.print("Devaddr: ");
    Serial.println(Devaddr, HEX);
    Serial.print("AppSKey: ");
    for (size_t i=0; i<sizeof(ArtKey); ++i) {
        if (i != 0) Serial.print("-");
        printHex2(ArtKey[i]);
    }
    Serial.print("\nNwkSKey: ");
    for (size_t i=0; i<sizeof(NwkKey); ++i) {
        if (i != 0) Serial.print("-");
        printHex2(NwkKey[i]);
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
            IsJoined = true;
            LMIC_setLinkCheckMode(0);
            Serial.println(F("EV_JOINED"));
            printJoiningInfo();
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
        LMIC_setTxData2(Port, DataPacket, PacketSize - 1, ConfirmedMode);
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

void LoRaWAN::setConfirmedMode(bool confirmedMode){
    _confirmedMode = confirmedMode;
}

void LoRaWAN::configure(uint16_t packetSize, uint8_t spreadFactor, uint32_t txPower, bool adrMode, bool checkLinkMode){
    // Initialize SPI
    _spiHandler->begin(_LoRaSck, _LoRaMiso, _LoRaMosi, _LoRaSs);

    // Allocate memory for data packet
    if(DataPacket != NULL){
        free(DataPacket);
    }
    DataPacket = (uint8_t*) malloc(packetSize * sizeof(uint8_t));
    PacketSize = packetSize;

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
    LMIC_setAdrMode(adrMode);
    
    // Disable link check validation, automatically enabled during join
    LMIC_setLinkCheckMode(checkLinkMode);

    // Set static session parameters. Instead of dynamically establishing a session
    LMIC_setDrTxpow(spreadFactor, txPower);
}

void LoRaWAN::loop(){
    // Process LMIC events
    os_runloop_once();
}

void LoRaWAN::iniciateJoin(){
    // If already joined, skip join procedure and set session keys directly
    if(IsJoined){
        Serial.println("Already joined, skipping join procedure");
        LMIC_setSession(Netid, Devaddr, NwkKey, ArtKey);
        return;
    }

    // Start joining procedure
    LMIC_startJoining();
}

void LoRaWAN::uplink(const uint8_t* data){
    memcpy(DataPacket, data, sizeof(DataPacket));
    do_send(&Sendjob);
}


int32_t LoRaWAN::getDownlinkSnr(){
    return LMIC.snr / 4;
}

int32_t LoRaWAN::getDownlinkRssi(){
    // Variables
    int32_t snr;
    int32_t rssi;

    // If using MCCI LMIC library, return rssi directly
    #ifdef MCCI_LMIC
        rssi = LMIC.rssi - RSSI_OFFSET;
        
    // Else, calculate rssi based on sx1272/76 datasheet
    #else
        // Get rssi adjust based on frequency  
        int32_t rssiAdjust;
        if(LMIC.freq > LORAWAN_SX1276_FREQ_LF_MAX){
            rssiAdjust = LORAWAN_SX1276_RSSI_ADJUST_HF;
        }
        else{
            rssiAdjust = LORAWAN_SX1276_RSSI_ADJUST_LF;   
        }

        // Get SNR
        snr = this->getDownlinkSnr();

        // Revert modification (applied in lmic/radio.c) to get PacketRssi.
        int32_t packetRssi = LMIC.rssi + 125 - LORAWAN_RSSI_OFFSET;
        if (snr < 0){
            rssi = rssiAdjust + packetRssi + snr;
        }
        else{
            rssi = rssiAdjust + (16 * packetRssi) / 15;
        }
    #endif

    return rssi;
}

bool LoRaWAN::connect(const char* server, int port){
    // Start join procedure
    Port = port;
    this->iniciateJoin();
    return IsJoined;
}

bool LoRaWAN::isConnected(){
    // Check if already joined in network
    return IsJoined;
}

void LoRaWAN::put(const char* resource, const char* payload){
    // Copy payload to data packet and send uplink
    uint8_t data[PacketSize];
    memcpy(data, payload, PacketSize);
    this->uplink(data);
}

void LoRaWAN::get(const char* resource, Tcallback callback){
    // Store callback for later use when downlink is received
    _storedCallback = callback;
}



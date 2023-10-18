#include <unicom_transmitter.h>

unsigned long generate_hash(char *str);

typedef struct {
    uint8_t wrapping_start[4];
    uint8_t transmitter[6];
    uint8_t receiver[6];
    uint8_t wrapping_end[8];
} header_t;

typedef struct {
    header_t header;
    uint8_t payload[MAX_DATA_SIZE];
} packet_t;

packet_t packet_data = {};
packet_t *packet = &packet_data;

/**
  * @brief     Initializes the transmitter
  *
  * @param      id                      name of the transmitter
  * @param      channel                 channel to use
  * @param      packet_sent_callback    callback after packet was sent   
  *
  */
uint8_t* init_transmitter(char *id, uint8_t channel, freedom_outside_cb_t packet_sent_callback) {
    // Default configuration stuff
    nvs_flash_init();
    tcpip_adapter_init();
    esp_event_loop_create_default();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_wifi_set_channel(channel, 0);
    if(packet_sent_callback != NULL) wifi_register_send_pkt_freedom_cb(packet_sent_callback);

    // Sets the frame control bits
    packet->header.wrapping_start[0] = 0x58;

    // Generate the hash identifier from the transmitter name/id and set it
    unsigned long hash = generate_hash(id);
    for(int i = 0; i < 3; i++) {
        packet->header.transmitter[i] = packet->header.transmitter[i+3] = ((hash >> (24-i*8)) & 0xFF);
    }

    uint8_t* payload_address = (uint8_t*)(&packet->payload);
    payload_address += 2;

    return payload_address;
}

/**
  * @brief      Transmits the packet
  *
  * @param      receiver            name of the receiver
  *
  */
esp_err_t transmit(char *receiver, unsigned short size) {
    //Set content size (Swap bytes because of endianness: little -> big)
    packet->payload[0] = ((uint8_t*)&size)[1];
    packet->payload[1] = ((uint8_t*)&size)[0];
    //Set the receiver id
    unsigned long hash = generate_hash(receiver);
    for(int i = 0; i < 3; i++) {
        packet->header.receiver[i] = packet->header.receiver[i+3] = ((hash >> (24 - i*8)) & 0xFF);
    }

    // Send the packet
    return esp_wifi_80211_tx(WIFI_IF_STA, packet, sizeof(packet->header) + size + 2, false);
}

/*
    Simple hash algorithm to generate an identifier
    http://www.cse.yorku.ca/~oz/hash.html
*/
unsigned long generate_hash(char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}
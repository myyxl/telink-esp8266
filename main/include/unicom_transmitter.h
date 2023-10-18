#ifndef UNICOM_TRANSMITTER_H
#define UNICOM_TRANSMITTER_H

#include <stdio.h>
#include <stdint.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <nvs_flash.h>

#define MAX_DATA_SIZE 2048

esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
typedef void (*freedom_outside_cb_t)(uint8_t status);
uint8_t* init_transmitter(char *id, uint8_t channel, freedom_outside_cb_t packet_sent_callback);
int wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
esp_err_t transmit(char* receiver, unsigned short size);

#endif
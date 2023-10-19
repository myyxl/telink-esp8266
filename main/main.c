/*
#include <unicom_transmitter.h>
*/
#include <stdio.h>
#include <bmp388.h>

void app_main() {

    bmp388_init();
    uint8_t chip_id;
    bmp388_read(0x00, &chip_id, 1);
    printf("%x\n", chip_id);

    /*
    // Initialize
    char message[] = "{\"altitude\": 10.15, \"velocity\": 4.5, \"acceleration\": 1, \"temperature\": 19.5}"; 
    uint8_t* buffer = init_transmitter("telink-esp8266", 0x07, NULL);

    // Fill buffer
    for(int i = 0; i < sizeof(message); i++) {
        buffer[i] = message[i];
    }

    esp_err_t code = transmit("telink-core", sizeof(message));
    if(code == ESP_OK) {
        ESP_LOGI("transmitter", "sent!");
    } else {
        ESP_LOGE("transmitter", "Error code: %x", code);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); */
}
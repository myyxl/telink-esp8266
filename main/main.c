#include <stdio.h>
#include <unicom_transmitter.h>
#include <esp_log.h>

void app_task() {
    //bmp388_handle handle = {};
    //bmp388_init(&handle);
    //bmp388_sensor_read(&handle);

    double altitute = 10.15f;
    double velocity = 4.5f;
    double acceleration = 1.0f;
    double temperature = 19.5f;

    uint8_t *altitute_ptr = (uint8_t*) &altitute;
    uint8_t *velocity_ptr = (uint8_t*) &velocity;
    uint8_t *acceleration_ptr = (uint8_t*) &acceleration;
    uint8_t *temperature_ptr = (uint8_t*) &temperature;
    
    // Init transmitter
    uint8_t* buffer = init_transmitter("telink-esp8266", 0x07, NULL);

    // Fill buffer
    for (int i = 0; i < 8; i++) {
        buffer[i] = altitute_ptr[i]; 
        buffer[i + 8] = velocity_ptr[i]; 
        buffer[i + 16] = acceleration_ptr[i]; 
        buffer[i + 24] = temperature_ptr[i]; 
    }

    // Send data in loop
    ESP_LOGI("transmitter", "Start transmitting");
    while(1) {
        esp_err_t code = transmit("telink-core", 4*8);
        if(code != ESP_OK) {
            ESP_LOGE("transmitter", "Error code: %x", code);
        }
        vTaskDelay(250 / portTICK_RATE_MS);
    }
}


void app_main() {
    xTaskCreate(app_task, "app_task", 2048, NULL, 10, NULL);
}
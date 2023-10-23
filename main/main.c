#include <stdio.h>
#include <unicom_transmitter.h>
#include <esp_log.h>
#include <driver_bmp388_basic.h>

void app_task() {
    bmp388_basic_init(BMP388_INTERFACE_IIC, BMP388_ADDRESS_ADO_LOW);
    vTaskDelay(2000 / portTICK_RATE_MS);

    float altitute = 10.15f;
    float velocity = 4.5f;
    float acceleration = 1.0f;
    float temperature = 19.5f;
    float pressure = 1.0f;

    uint8_t *altitute_ptr = (uint8_t*) &altitute;
    uint8_t *velocity_ptr = (uint8_t*) &velocity;
    uint8_t *acceleration_ptr = (uint8_t*) &acceleration;
    uint8_t *temperature_ptr = (uint8_t*) &temperature;
    
    // Init transmitter
    uint8_t* buffer = init_transmitter("telink-esp8266", 0x07, NULL);

    // Send data in loop
    ESP_LOGI("transmitter", "Start transmitting");
    while(1) {
        bmp388_basic_read((float *)&temperature, (float *)&pressure);
        for (int i = 0; i < 8; i++) {
            buffer[i] = altitute_ptr[i]; 
            buffer[i + 4] = velocity_ptr[i]; 
            buffer[i + 8] = acceleration_ptr[i]; 
            buffer[i + 16] = temperature_ptr[i]; 
        }
        esp_err_t code = transmit("telink-core", 16);
        if(code != ESP_OK) {
            ESP_LOGE("transmitter", "Error code: %x", code);
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void app_main() {
    xTaskCreate(app_task, "app_task", 2048, NULL, 10, NULL);
}
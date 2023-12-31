#include <stdio.h>
#include <unicom_transmitter.h>
#include <esp_log.h>
#include <driver_bmp388_basic.h>
#include <math.h>

void app_task() {
    ESP_LOGI("bmp388", "Initializing sensor");
    bmp388_basic_init(BMP388_INTERFACE_IIC, BMP388_ADDRESS_ADO_LOW);
    vTaskDelay(2000 / portTICK_RATE_MS);

    float altitude = 0.0f;
    float velocity = 0.0f;
    float acceleration = 0.0f;
    float temperature = 0.0f;
    float pressure = 0.0f;

    float last_altitude = 0.0f;
    float last_velocity = 0.0f;

    uint8_t *altitude_ptr = (uint8_t*) &altitude;
    uint8_t *velocity_ptr = (uint8_t*) &velocity;
    uint8_t *acceleration_ptr = (uint8_t*) &acceleration;
    uint8_t *temperature_ptr = (uint8_t*) &temperature;

    float base_altitude = 0.0f;

    ESP_LOGI("bmp388", "Calibrating base altitude");
    for (int i = 0; i < 30; i++) {
        float reading = 0.0f;
        bmp388_basic_read((float *)&temperature, (float *)&reading);
        base_altitude += 44330 * (1 - pow((reading/101325), (1/5.255)));
        ESP_LOGI("bmp388", "Reading[%d/30]: %d", i+1, (int) reading);
        vTaskDelay(1500 / portTICK_RATE_MS);
    }
    base_altitude = base_altitude / 30;
    ESP_LOGI("bmp388", "Base altitude: %d", (int) base_altitude);

    // Init transmitter
    uint8_t* buffer = init_transmitter("telink-esp8266", 0x07, NULL);

    // Send data in loop
    ESP_LOGI("transmitter", "Start transmitting");
    while(1) {
        bmp388_basic_read((float *)&temperature, (float *)&pressure);

        altitude = 44330 * (1 - pow((pressure/101325), (1/5.255))) - base_altitude;
        if(altitude < 0) altitude = 0.0f;
        if(last_altitude != 0.0f) velocity = altitude - last_altitude;
        if(last_velocity != 0.0f) acceleration = velocity - last_velocity;
        if(velocity < 0) velocity = 0.0f;
        if(acceleration < 0) acceleration = 0.0f;

        for (int i = 0; i < 4; i++) {
            buffer[i] = altitude_ptr[i]; 
            buffer[i + 4] = velocity_ptr[i]; 
            buffer[i + 8] = acceleration_ptr[i]; 
            buffer[i + 12] = temperature_ptr[i]; 
        }
        
        esp_err_t code = transmit("telink-core", 16);
        if(code != ESP_OK) {
            ESP_LOGE("transmitter", "Error code: %x", code);
        }
        last_altitude = altitude;
        last_velocity = velocity;
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void app_main() {
    xTaskCreate(app_task, "app_task", 2048, NULL, 10, NULL);
}
#include <bmp388.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <math.h>
#include <inttypes.h>

void bmp388_init(bmp388_handle *handle) {

    // Configuration
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = 14;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = 2;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300;

    i2c_driver_install(I2C_NUM_0, conf.mode);
    i2c_param_config(I2C_NUM_0, &conf);

    //Verify chip id
    uint8_t chip_id;
    bmp388_read(0x00, &chip_id, 1);

    if(chip_id == 0x50) {
        ESP_LOGI("bmp388", "Chip id ok");
    } else {
        ESP_LOGE("bmp388", "Unknown chip id 0x%x", chip_id);
    }

    // Get calibration data
    bmp388_calibration_t cal = handle->cal;
    bmp388_read_coefficients(&cal);
} 

void bmp388_read(uint8_t reg, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SENSOR_ADDR << 1) | I2C_MASTER_WRITE, 0x1);
    i2c_master_write_byte(cmd, reg, 0x1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SENSOR_ADDR << 1) | I2C_MASTER_READ, 0x1);
    i2c_master_read(cmd, data, len, 0x2);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}

void bmp388_write(uint8_t reg, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SENSOR_ADDR << 1) | I2C_MASTER_WRITE, 0x1);
    i2c_master_write_byte(cmd, reg, 0x1);
    i2c_master_write(cmd, data, len, 0x1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}

void bmp388_read_coefficients(bmp388_calibration_t *calibration_data) {

    ESP_LOGI("bmp388", "Reading coefficients");

    u_int16_t t1, t2, p5, p6;
    int16_t p1, p2, p9;
    int8_t t3, p3, p4, p7, p8, p10, p11;

    // Temperature
    bmp388_read(0x31, (uint8_t*)&t1, 2);
    bmp388_read(0x33, (uint8_t*)&t2, 2);
    bmp388_read(0x35, (uint8_t*)&t3, 1);

    // Pressure
    bmp388_read(0x36, (uint8_t*)&p1, 2);
    bmp388_read(0x38, (uint8_t*)&p2, 2);
    bmp388_read(0x3A, (uint8_t*)&p3, 1);
    bmp388_read(0x3B, (uint8_t*)&p4, 1);
    bmp388_read(0x3C, (uint8_t*)&p5, 2);
    bmp388_read(0x3E, (uint8_t*)&p6, 2);
    bmp388_read(0x40, (uint8_t*)&p7, 1);
    bmp388_read(0x41, (uint8_t*)&p8, 1);
    bmp388_read(0x42, (uint8_t*)&p9, 2);
    bmp388_read(0x44, (uint8_t*)&p10, 1);
    bmp388_read(0x45, (uint8_t*)&p11, 1);

    // Temperature
    double temp_var;
    temp_var = 0.00390625f;
    calibration_data->t1 = ((double)t1 / temp_var);
    temp_var = 1073741824.0f;
    calibration_data->t2 = ((double)t2 / temp_var);
    temp_var = 281474976710656.0f;
    calibration_data->t3 = ((double)t3 / temp_var);


    // Soft reset
    // uint8_t ctrl = 0xB6;
    // bmp388_write(0x7E, &ctrl, 1);

    // Force enable
    uint8_t ctrl = 0x13;
    bmp388_write(0x1B, &ctrl, 1);

    // Hacky wait
    for(int i = 0; i < 1000; i++) {
        uint8_t status;
        bmp388_read(0x3, &status, 1);
        if((status & 0x60 )== 0x60) break;
    }

    // Read temperature
    uint32_t data_xlsb;
    uint32_t data_lsb;
    uint32_t data_msb;
    bmp388_read(0x07, (uint8_t*)&data_xlsb, 1);
    bmp388_read(0x08, (uint8_t*)&data_lsb, 1);
    bmp388_read(0x09, (uint8_t*)&data_msb, 1);

    data_lsb = data_lsb << 8;
    data_msb = data_msb << 16;

    int64_t t_data = data_msb | data_lsb | data_xlsb;

    ESP_LOGI("bmp388", "temp: %d", (int) t_data);

    double partial_data1;
    double partial_data2;

    partial_data1 = (double)(t_data - calibration_data->t1);
    partial_data2 = (double)(partial_data1 * calibration_data->t2);
    double temp = partial_data2 + (partial_data1 * partial_data1) * calibration_data->t3;


    ESP_LOGI("bmp388", "Current temperature: %d", (int) temp);
}
/*
void bmp388_sensor_read(bmp388_handle *handle) {
    // Force enable
    uint8_t ctrl = 0x13;
    bmp388_write(0x1B, &ctrl, 1);

    // Hacky wait
    for(int i = 0; i < 1000; i++) {
        uint8_t status;
        bmp388_read(0x3, &status, 1);
        if((status & 0x60 )== 0x60) break;
    }

    // Read temperature
    uint64_t t_data;
    bmp388_read(0x07, (uint8_t*)&t_data, 3);
    ESP_LOGI("bmp388", "temp: %d", (int) t_data);

    double partial_data1;
    double partial_data2;

    partial_data1 = (double)(t_data - handle->cal.t1);
    partial_data2 = (double)(partial_data1 * handle->cal.t2);
    handle->temperature = partial_data2 + (partial_data1 * partial_data1) * handle->cal.t3;
}*/
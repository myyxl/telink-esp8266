#ifndef BMP388_H
#define BMP388_H

#include "driver/i2c.h"

#define SENSOR_ADDR 0x77

typedef struct {
    double t1;
    double t2;
    double t3;
} bmp388_calibration_t;

typedef struct {
    bmp388_calibration_t cal;
    double temperature;
} bmp388_handle;

void bmp388_init(bmp388_handle *handle);
void bmp388_read(uint8_t register, uint8_t *data, size_t len);
void bmp388_write(uint8_t reg, uint8_t *data, size_t len);
void bmp388_read_coefficients(bmp388_calibration_t *calibration_data);
void bmp388_sensor_read(bmp388_handle *handle);

#endif
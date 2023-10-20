#ifndef BMP388_H
#define BMP388_H

#include "driver/i2c.h"

#define SENSOR_ADDR 0x77

typedef struct {
    uint64_t t1;
    uint64_t t2;
    uint64_t t3;
} bmp388_calibration_t;

void bmp388_init();
void bmp388_read(uint8_t register, uint8_t *data, size_t len);
void bmp388_write(uint8_t reg, uint8_t *data, size_t len);
void bmp388_read_coefficients(bmp388_calibration_t *calibration_data);
void bmp388_sensor_read(bmp388_calibration_t *cal_data);

#endif
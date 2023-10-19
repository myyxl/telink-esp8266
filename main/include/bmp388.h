#ifndef BMP388_H
#define BMP388_H

#include "driver/i2c.h"

#define SENSOR_ADDR 0x77

void bmp388_init();
void bmp388_read(uint8_t register, uint8_t *data, size_t len);

#endif
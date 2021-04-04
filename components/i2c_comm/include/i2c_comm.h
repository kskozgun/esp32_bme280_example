#include <stdio.h>
#include "esp_system.h"
#include "driver/i2c.h"

void i2c_init(uint32_t port_no, i2c_config_t *i2c_config);

int8_t bme280_i2c_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint32_t len);

int8_t bme280_i2c_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint32_t len);


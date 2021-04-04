#include <stdio.h>
#include "i2c_comm.h"

void i2c_init(uint32_t port_no, i2c_config_t *i2c_config)
{
    i2c_param_config(port_no, i2c_config);
    
    i2c_driver_install(port_no, I2C_MODE_MASTER, 0, 0, 0); // TODO: Last parameter have to be defined
}

int8_t bme280_i2c_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint32_t len)
{
    esp_err_t esp_err = 0;
    i2c_cmd_handle_t i2c_handle;

    i2c_handle = i2c_cmd_link_create();

    i2c_master_start(i2c_handle);

    esp_err += i2c_master_write_byte(i2c_handle, i2c_addr, true);

    esp_err += i2c_master_write_byte(i2c_handle, reg_addr, true);

    esp_err += i2c_master_start(i2c_handle);

    esp_err += i2c_master_write_byte(i2c_handle, (i2c_addr + 1), true);

    for(int i = 0; i < len - 1; i++) {
        esp_err += i2c_master_read_byte(i2c_handle, reg_data, I2C_MASTER_ACK);
        reg_data++;
    }

    esp_err += i2c_master_read_byte(i2c_handle, reg_data, I2C_MASTER_NACK);

    esp_err += i2c_master_stop(i2c_handle);

    esp_err += i2c_master_cmd_begin(I2C_NUM_0, i2c_handle, 100);

    i2c_cmd_link_delete(i2c_handle);

    if(esp_err) {
        return -1;
    }

    return 0;
}

int8_t bme280_i2c_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint32_t len)
{
    esp_err_t esp_err = 0;
    i2c_cmd_handle_t i2c_handle;

    i2c_handle = i2c_cmd_link_create();

    i2c_master_start(i2c_handle);

    esp_err += i2c_master_write_byte(i2c_handle, i2c_addr, true);

    for (int i = 0; i < len; i++)
    {
        esp_err += i2c_master_write_byte(i2c_handle, reg_addr, true);

        esp_err += i2c_master_write_byte(i2c_handle, *reg_data, true);

        reg_addr++;
        reg_data++;        
    }

    esp_err += i2c_master_stop(i2c_handle);

    esp_err += i2c_master_cmd_begin(I2C_NUM_0, i2c_handle, 100);

    i2c_cmd_link_delete(i2c_handle);

    return 0;
}

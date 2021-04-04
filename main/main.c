/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "i2c_comm.h"
#include "i2c_conf.h"
#include "bme280.h"


bme280_dev_t bme280_dev;

bme280_trim_data_t bme280_trim_data;


void app_main(void)
{
    bme280_err_t bme280_err;

    i2c_init(I2C_NUM_0, &conf);

    bme280_err = bme280_init(&bme280_dev, BME280_TEMP_OVER_1, BME280_HUM_OVER_1, BME280_PRESS_OVER_1);
    if(bme280_err) {
        printf("Error with number: %d\n", bme280_err);
    }

    bme280_err = bme280_set_mode(BME280_MOD_NORMAL);
    if(bme280_err) {
        printf("Error with number: %d\n", bme280_err);
    }

    while(1)
    {
        bme280_read_sensor_data(&bme280_dev);
        vTaskDelay(pdMS_TO_TICKS(1000));  
        printf("Pressure is %f\n", bme280_dev.pressure);
        printf("Temp is %f\n", bme280_dev.temperature);
        printf("Humid is %f\n", bme280_dev.humidity);
    }
}

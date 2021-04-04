/**
 * @file bme280.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdint.h>

/* Memory address */
#define BME280_REG_ID 0xD0
#define BME280_REG_TEMP_TRIM 0x88
#define BME280_REG_HUM_1_TRIM 0xA1
#define BME280_REG_HUM_2_TRIM 0xE1
#define BME280_REG_TEMP 0xFA
#define BME280_REG_PRESS 0xF7
#define BME280_REG_HUM 0xFD
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CTRL_HUM 0xF2

#define BME280_TEMP_TRIM_LEN 6
#define BME280_TEMP_PRES_TRIM_LEN 24
#define BME280_HUM_1_TRIM_LEN 1
#define BME280_HUM_2_TRIM_LEN 8 
#define BME280_TEMP_LEN 3
#define BME280_PRESS_LEN 3
#define BME280_HUM_LEN 2
#define BME280_CTRL_MEAS_LEN 1
#define BME280_CTRL_HUM_LEN 1

#define BME280_HUM_4_TRIM_MASK 0x0F
#define BME280_HUM_5_TRIM_MASK 0x0F
#define BME280_HUM_TEMP_READ_MASK 0xF0
#define BME280_MOD_SLEEP_MASK 0xFC
#define BME280_CTRL_MEAS_TEMP_MASK 0x1F
#define BME280_CTRL_PRESS_TEMP_MASK 0xE3
#define BME280_CTRL_HUM_MASK 0xF8

#define BME280_MOD_SLEEP 0x00
#define BME280_MOD_NORMAL 0x03
#define BME280_MOD_FORCED 0x01

/* */
#define BME280_TEMP_SKIP 0x00
#define BME280_TEMP_OVER_1 0x01
#define BME280_TEMP_OVER_2 0x02
#define BME280_TEMP_OVER_4 0x03
#define BME280_TEMP_OVER_8 0x04
#define BME280_TEMP_OVER_16 0x05

/* */
#define BME280_PRESS_SKIP 0x00
#define BME280_PRESS_OVER_1 0x01
#define BME280_PRESS_OVER_2 0x02
#define BME280_PRESS_OVER_4 0x03
#define BME280_PRESS_OVER_8 0x04
#define BME280_PRESS_OVER_16 0x05

/* */
#define BME280_HUM_SKIP 0x00
#define BME280_HUM_OVER_1 0x01
#define BME280_HUM_OVER_2 0x02
#define BME280_HUM_OVER_4 0x03
#define BME280_HUM_OVER_8 0x04
#define BME280_HUM_OVER_16 0x05

#define BME280_ID 0x60
#define BME280_SLV_ADDR (0x76 << 1)


typedef enum {
    BME280_OK = 0U,
    BME280_WRNG_DEV,
    BME280_I2C_ERR

} bme280_err_t;

typedef struct {
    uint16_t t1;
    int16_t  t2;
    int16_t  t3;
    uint16_t p1;
    int16_t  p2;
    int16_t  p3;
    int16_t  p4;
    int16_t  p5;
    int16_t  p6;
    int16_t  p7;
    int16_t  p8;
    int16_t  p9;
    uint8_t  h1;
    int16_t  h2;
    uint8_t  h3;
    int16_t  h4;
    int16_t  h5;
    int8_t  h6;
} bme280_trim_data_t;

typedef struct {
    uint8_t i2c_addr;
    uint8_t device_id;
    bme280_trim_data_t bme280_trim_data;
    float temperature;
    float humidity;
    float pressure;
} bme280_dev_t;

bme280_err_t bme280_set_mode(uint8_t sensor_mode);

bme280_err_t bme280_init(bme280_dev_t *dev, uint8_t temp_samp, uint8_t hum_samp, uint8_t press_samp);

bme280_err_t bme280_compensate_temp(bme280_dev_t *dev);

bme280_err_t bme280_compensate_hum(bme280_dev_t *dev);

bme280_err_t bme280_compensate_press(bme280_dev_t *dev);

bme280_err_t bme280_read_sensor_data(bme280_dev_t *dev);








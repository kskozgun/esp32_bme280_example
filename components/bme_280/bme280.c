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

#include <stdio.h>
#include "bme280.h"
#include "i2c_comm.h"

static int32_t t_fine = 0;

static bme280_err_t bme280_oversamp_temp(uint8_t oversamp_mode);
static bme280_err_t bme280_oversamp_hum(uint8_t oversamp_mode);
static bme280_err_t bme280_oversamp_press(uint8_t oversamp_mode);



bme280_err_t bme280_set_mode(uint8_t sensor_mode)
{
    int8_t i2c_ret;
    uint8_t status;

    i2c_ret = bme280_i2c_read(BME280_SLV_ADDR, BME280_REG_CTRL_MEAS, &status, BME280_CTRL_MEAS_LEN);
    if (i2c_ret)
    {
        return BME280_I2C_ERR;
    }

    status &= BME280_MOD_SLEEP_MASK;

    status += sensor_mode;

    i2c_ret = bme280_i2c_write(BME280_SLV_ADDR, BME280_REG_CTRL_MEAS, &status, BME280_CTRL_MEAS_LEN);
    if (i2c_ret)
    {
        return BME280_I2C_ERR;
    }

    return BME280_OK;
}

static bme280_err_t bme280_read_trim_data(bme280_trim_data_t *bme280_trim_data)
{
    int8_t i2c_ret;
    uint8_t temp_buffer[24] = {0};

    i2c_ret = bme280_i2c_read(BME280_SLV_ADDR, BME280_REG_TEMP_TRIM, temp_buffer, BME280_TEMP_PRES_TRIM_LEN);
    if (i2c_ret)
    {
        return BME280_I2C_ERR;
    }

    bme280_trim_data->t1 = ((uint16_t)(temp_buffer[1] << 8) | temp_buffer[0]);
    bme280_trim_data->t2 = (int16_t)(temp_buffer[3] << 8)   | temp_buffer[2];
    bme280_trim_data->t3 = (uint16_t)(temp_buffer[5] << 8)  | temp_buffer[4];

    bme280_trim_data->p1 = (uint16_t)(temp_buffer[7] << 8) | temp_buffer[6];
    bme280_trim_data->p2 = (int16_t)(temp_buffer[9] << 8)  | temp_buffer[8];
    bme280_trim_data->p3 = (int16_t)(temp_buffer[11] << 8) | temp_buffer[10];
    bme280_trim_data->p4 = (int16_t)(temp_buffer[13] << 8) | temp_buffer[12];
    bme280_trim_data->p5 = (int16_t)(temp_buffer[15] << 8) | temp_buffer[14];
    bme280_trim_data->p6 = (int16_t)(temp_buffer[17] << 8) | temp_buffer[16];
    bme280_trim_data->p7 = (int16_t)(temp_buffer[19] << 8) | temp_buffer[18];
    bme280_trim_data->p8 = (int16_t)(temp_buffer[21] << 8) | temp_buffer[20];
    bme280_trim_data->p9 = (int16_t)(temp_buffer[23] << 8) | temp_buffer[22];

    i2c_ret = bme280_i2c_read(BME280_SLV_ADDR, BME280_REG_HUM_1_TRIM, temp_buffer, BME280_HUM_1_TRIM_LEN);
    if (i2c_ret)
    {
        return BME280_I2C_ERR;
    }

    i2c_ret = bme280_i2c_read(BME280_SLV_ADDR, BME280_REG_HUM_2_TRIM, &temp_buffer[1], BME280_HUM_2_TRIM_LEN);
    if (i2c_ret)
    {
        return BME280_I2C_ERR;
    }

    bme280_trim_data->h1 = temp_buffer[0];
    bme280_trim_data->h2 = (int16_t)(temp_buffer[2] << 8) | temp_buffer[1];
    bme280_trim_data->h3 = temp_buffer[3];
    bme280_trim_data->h4 = (int16_t)(temp_buffer[4] << 4) | ((int16_t)temp_buffer[5] & BME280_HUM_4_TRIM_MASK);
    bme280_trim_data->h5 = (int16_t)(temp_buffer[7] << 4) | ((int16_t)temp_buffer[6] & BME280_HUM_5_TRIM_MASK);
    bme280_trim_data->h6 = temp_buffer[8];

    return BME280_OK;
}

bme280_err_t bme280_init(bme280_dev_t *dev)
{
    bme280_err_t ret;
    int8_t i2c_ret;
    i2c_ret = bme280_i2c_read(BME280_SLV_ADDR, BME280_REG_ID, &dev->device_id, 1);
    if (i2c_ret){
        return BME280_I2C_ERR;
    }

    if (BME280_ID != dev->device_id){
        return BME280_WRNG_DEV;
    }

    // Soft Reset
    ret = bme280_read_trim_data(&dev->bme280_trim_data);
    if(ret){
        return ret;
    }

    ret += bme280_oversamp_temp(dev->oversampling_temp);
    ret += bme280_oversamp_press(dev->oversampling_pres);
    ret += bme280_oversamp_hum(dev->oversampling_hum);
    if(ret) {
        return BME280_I2C_ERR;
    }
    return BME280_OK;
}   

bme280_err_t bme280_compensate_temp(bme280_dev_t *dev)
{   
    int8_t i2c_ret;
    uint8_t temp_buffer[3] = {0};
    int32_t var1,var2;
    int32_t adc_T;
    
    i2c_ret = bme280_i2c_read(BME280_SLV_ADDR, BME280_REG_TEMP, temp_buffer, BME280_TEMP_LEN);
    if (i2c_ret)
    {
        return BME280_I2C_ERR;
    }
    
    adc_T = ((int32_t)(temp_buffer[2] & BME280_HUM_TEMP_READ_MASK)) >> 4;
    adc_T |= (int32_t)(temp_buffer[0] << 12) | (int32_t)(temp_buffer[1] << 4);

    var1 = (((adc_T >> 3) - (dev->bme280_trim_data.t1 << 1)) * (dev->bme280_trim_data.t2)) >> 11;
    var2 = ((((adc_T >> 4) - (dev->bme280_trim_data.t1)) * ((adc_T >> 4) - dev->bme280_trim_data.t1)) >> 12)
                             * (dev->bme280_trim_data.t3) >> 14;

    t_fine = var1 + var2; 
    var1 = (((t_fine) * 5) + 128) >> 8;

    dev->temperature = (float)(var1) / 100;
    return BME280_OK;
}

bme280_err_t bme280_compensate_hum(bme280_dev_t *dev)
{
    int32_t temp_1;
    int32_t temp_2;
    int32_t temp_3;
    int32_t temp_4;
    int32_t temp_5;
    uint32_t humidity_max = 102400;
    uint8_t temp_buffer[2] = {0};    
    int32_t adc_H;
    int8_t i2c_ret;

    i2c_ret = bme280_i2c_read(BME280_SLV_ADDR, BME280_REG_HUM, temp_buffer, BME280_HUM_LEN);
    if (i2c_ret){
        return BME280_I2C_ERR;
    }

    adc_H = ((int32_t)(temp_buffer[0] << 8) | temp_buffer[1]);

    temp_1 = t_fine - ((int32_t)76800);
    temp_2 = (int32_t)(adc_H << 14);
    temp_3 = (int32_t)(((int32_t)dev->bme280_trim_data.h4) << 20);
    temp_4 = ((int32_t)dev->bme280_trim_data.h5) * temp_1;
    temp_5 = (((temp_2 - temp_3) - temp_4) + (int32_t)16384) >> 15;
    temp_2 = (temp_1 * ((int32_t)dev->bme280_trim_data.h6)) >> 10;
    temp_3 = (temp_1 * ((int32_t)dev->bme280_trim_data.h3)) >> 11;
    temp_4 = ((temp_2 * (temp_3 + (int32_t)32768)) >> 10) + (int32_t)2097152;
    temp_2 = ((temp_4 * ((int32_t)dev->bme280_trim_data.h2)) + 8192) >> 14;
    temp_3 = temp_5 * temp_2;
    temp_4 = ((temp_3 / 32768) * (temp_3 / 32768)) >> 7;
    temp_5 = temp_3 - ((temp_4 * ((int32_t)dev->bme280_trim_data.h1)) >> 4);
    
    if(temp_5 < 0){
        temp_5 = 0;
    }

    if(temp_5 > 419430400){
        temp_5 = 419430400;
    }
    temp_5 = (uint32_t)(temp_5 >> 12);
    dev->humidity = (float)(temp_5) / 1024;

    if (dev->humidity > humidity_max)
    {
        dev->humidity = humidity_max;
    }
    return BME280_OK;
}

bme280_err_t bme280_compensate_press(bme280_dev_t *dev)
{
    int8_t i2c_ret;
    uint8_t temp_buffer[3];
    int32_t adc_P = 0; 
    int64_t var1;
    int64_t var2;
    int64_t var3;
    int64_t var4;

    i2c_ret = bme280_i2c_read(BME280_SLV_ADDR, BME280_REG_PRESS, temp_buffer, BME280_PRESS_LEN);
    if (i2c_ret){
        return BME280_I2C_ERR;
    }

    adc_P = (temp_buffer[2] & BME280_HUM_TEMP_READ_MASK) >> 4;
    adc_P |= (((int32_t)(temp_buffer[0])) << 12) | ((int32_t)(temp_buffer[1]) << 4);

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dev->bme280_trim_data.p6;
    var2 = var2 + ((var1 * (int64_t)dev->bme280_trim_data.p5) * 131072);
    var2 = var2 + (((int64_t)dev->bme280_trim_data.p4) * 34359738368);
    var1 = ((var1 * var1 * (int64_t)dev->bme280_trim_data.p3) / 256) + ((var1 * ((int64_t)dev->bme280_trim_data.p2) * 4096));
    var3 = ((int64_t)1) * 140737488355328;
    var1 = (var3 + var1) * ((int64_t)dev->bme280_trim_data.p1) / 8589934592;

    /* To avoid divide by zero exception */
    if (var1 != 0) {
        var4 = 1048576 - adc_P;
        var4 = (((var4 * INT64_C(2147483648)) - var2) * 3125) / var1;
        var1 = (((int64_t)dev->bme280_trim_data.p9) * (var4 / 8192) * (var4 / 8192)) / 33554432;
        var2 = (((int64_t)dev->bme280_trim_data.p8) * var4) / 524288;
        var4 = ((var4 + var1 + var2) / 256) + (((int64_t)dev->bme280_trim_data.p7) * 16);

        dev->pressure = ((float)(var4)) / 256;
    } else {
        dev->pressure = 0.0;
    }
    return 0;
}

bme280_err_t bme280_read_sensor_data(bme280_dev_t *dev)
{
    bme280_err_t ret;

    ret = bme280_compensate_temp(dev);
    if(ret){
        return ret;
    }

    ret = bme280_compensate_hum(dev);
    if(ret){
        return ret;
    }

    ret = bme280_compensate_press(dev);
    if(ret){
        return ret;
    }

    return BME280_OK;
}


static bme280_err_t bme280_oversamp_temp(uint8_t oversamp_mode)
{
    int8_t i2c_ret;   
    uint8_t mode;   

    i2c_ret = bme280_i2c_read(BME280_SLV_ADDR, BME280_REG_CTRL_MEAS, &mode, BME280_CTRL_MEAS_LEN);
    if (i2c_ret){
        return BME280_I2C_ERR;
    }

    mode &= BME280_CTRL_MEAS_TEMP_MASK;

    mode |= oversamp_mode << 5;

    i2c_ret = bme280_i2c_write(BME280_SLV_ADDR, BME280_REG_CTRL_MEAS, &mode, BME280_CTRL_MEAS_LEN);
    if (i2c_ret){
        return BME280_I2C_ERR;
    }
    return BME280_OK;
}

static bme280_err_t bme280_oversamp_press(uint8_t oversamp_mode)
{
    int8_t i2c_ret;   
    uint8_t mode;   

    i2c_ret = bme280_i2c_read(BME280_SLV_ADDR, BME280_REG_CTRL_MEAS, &mode, BME280_CTRL_MEAS_LEN);
    if (i2c_ret){
        return BME280_I2C_ERR;
    }

    mode &= BME280_CTRL_PRESS_TEMP_MASK;

    mode |= oversamp_mode << 2;

    i2c_ret = bme280_i2c_write(BME280_SLV_ADDR, BME280_REG_CTRL_MEAS, &mode, BME280_CTRL_MEAS_LEN);
    if (i2c_ret){
        return BME280_I2C_ERR;
    }
    return BME280_OK;
}

static bme280_err_t bme280_oversamp_hum(uint8_t oversamp_mode)
{
    int8_t i2c_ret;   
    uint8_t mode;   

    i2c_ret = bme280_i2c_read(BME280_SLV_ADDR, BME280_REG_CTRL_HUM, &mode, BME280_CTRL_HUM_LEN);
    if (i2c_ret){
        return BME280_I2C_ERR;
    }

    mode &= BME280_CTRL_HUM_MASK;

    mode |= oversamp_mode;

    i2c_ret = bme280_i2c_write(BME280_SLV_ADDR, BME280_REG_CTRL_HUM, &mode, BME280_CTRL_HUM_LEN);
    if (i2c_ret){
        return BME280_I2C_ERR;
    }
    return BME280_OK;
}


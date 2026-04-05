#ifndef HW_SENSOR_BME280_H
#define HW_SENSOR_BME280_H

#include "qom/object.h"
#include "hw/i2c/i2c.h"

/* BME280 registers */
#define BME280_REG_ID           0xD0
#define BME280_REG_RESET        0xE0
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_STATUS       0xF3
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_PRESS_MSB    0xF7
#define BME280_REG_PRESS_LSB    0xF8
#define BME280_REG_PRESS_XLSB   0xF9
#define BME280_REG_TEMP_MSB     0xFA
#define BME280_REG_TEMP_LSB     0xFB
#define BME280_REG_TEMP_XLSB    0xFC
#define BME280_REG_HUM_MSB      0xFD
#define BME280_REG_HUM_LSB      0xFE

/* Calibration registers */
#define BME280_REG_CALIB_START  0x88
#define BME280_REG_CALIB_H1     0xA1
#define BME280_REG_CALIB_H2     0xE1

#define BME280_CHIP_ID          0x60
#define BME280_SOFT_RESET_VAL   0xB6

#define TYPE_BME280 "bme280"
OBJECT_DECLARE_SIMPLE_TYPE(BME280State, BME280)

struct BME280State {
    I2CSlave i2c;

    uint8_t regs[256];   /* 所有暫存器 */
    uint8_t reg_addr;    /* 目前暫存器位址 */
    uint8_t len;         /* tx 收到幾個 byte */
};

#endif

#include "qemu/osdep.h"
#include "hw/i2c/i2c.h"
#include "hw/sensor/bme280.h"
#include "qemu/module.h"
#include "qom/object.h"

static void bme280_reset(BME280State *s)
{
    memset(s->regs, 0, sizeof(s->regs));

    /* chip id */
    s->regs[BME280_REG_ID] = BME280_CHIP_ID;

    /* 校正資料：放固定的預設值讓 driver 可以換算 */
    s->regs[0x88] = 0x70; s->regs[0x89] = 0x6B; /* T1 = 0x6B70 */
    s->regs[0x8A] = 0x43; s->regs[0x8B] = 0x67; /* T2 = 0x6743 */
    s->regs[0x8C] = 0x18; s->regs[0x8D] = 0x00; /* T3 = 0x0018 */

    /* 溫度原始資料：對應約 25°C */
    s->regs[BME280_REG_TEMP_MSB]  = 0x7E;
    s->regs[BME280_REG_TEMP_LSB]  = 0x90;
    s->regs[BME280_REG_TEMP_XLSB] = 0x00;

    /* 氣壓原始資料 */
    s->regs[BME280_REG_PRESS_MSB]  = 0x51;
    s->regs[BME280_REG_PRESS_LSB]  = 0x9A;
    s->regs[BME280_REG_PRESS_XLSB] = 0x00;

    /* 濕度原始資料 */
    s->regs[BME280_REG_HUM_MSB] = 0x6E;
    s->regs[BME280_REG_HUM_LSB] = 0x00;

    s->reg_addr = 0;
    s->len = 0;
}

static uint8_t bme280_rx(I2CSlave *i2c)
{
    BME280State *s = BME280(i2c);
    uint8_t val = s->regs[s->reg_addr];
    printf("[BME280] READ reg=0x%02x val=0x%02x\n", s->reg_addr, val);
    fflush(stdout);
    s->reg_addr++;  /* 自動遞增 */
    return val;
}

static int bme280_tx(I2CSlave *i2c, uint8_t data)
{
    BME280State *s = BME280(i2c);

    if (s->len == 0) {
        /* 第一個 byte 是暫存器位址 */
        s->reg_addr = data;
        s->len++;
    } else {
        /* 之後的 byte 是寫入資料 */
        printf("[BME280] WRITE reg=0x%02x val=0x%02x\n", s->reg_addr, data);
        fflush(stdout);
        if (data == BME280_SOFT_RESET_VAL && s->reg_addr == BME280_REG_RESET) {
            bme280_reset(s);
    printf("[BME280] realize called!\n"); fflush(stdout);
        } else {
            s->regs[s->reg_addr] = data;
        }
        s->reg_addr++;
    }
    return 0;
}

static int bme280_event(I2CSlave *i2c, enum i2c_event event)
{
    BME280State *s = BME280(i2c);

    if (event == I2C_START_SEND || event == I2C_START_RECV) {
        s->len = 0;
    }
    return 0;
}

static void bme280_realize(DeviceState *dev, Error **errp)
{
    BME280State *s = BME280(dev);
    bme280_reset(s);
    printf("[BME280] realize called!\n"); fflush(stdout);
}

static void bme280_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    I2CSlaveClass *k = I2C_SLAVE_CLASS(klass);

    dc->realize = bme280_realize;
    k->event = bme280_event;
    k->recv = bme280_rx;
    k->send = bme280_tx;
}

static const TypeInfo bme280_info = {
    .name          = TYPE_BME280,
    .parent        = TYPE_I2C_SLAVE,
    .instance_size = sizeof(BME280State),
    .class_init    = bme280_class_init,
};

static void bme280_register_types(void)
{
    type_register_static(&bme280_info);
}
type_init(bme280_register_types)

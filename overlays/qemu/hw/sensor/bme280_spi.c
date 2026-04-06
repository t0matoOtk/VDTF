#include "qemu/osdep.h"
#include "hw/ssi/ssi.h"
#include "hw/sensor/bme280.h"
#include "qemu/module.h"
#include "qom/object.h"

#define TYPE_BME280_SPI "bme280-spi"
OBJECT_DECLARE_SIMPLE_TYPE(BME280SPIState, BME280_SPI)

struct BME280SPIState {
    SSIPeripheral parent_obj;

    uint8_t regs[256];
    uint8_t reg_addr;
    bool    reading;     /* 目前是讀取模式 */
    int     byte_count;  /* 這次傳輸收到第幾個 byte */
};

static void bme280_spi_reset(BME280SPIState *s)
{
    memset(s->regs, 0, sizeof(s->regs));

    /* chip id */
    s->regs[BME280_REG_ID] = BME280_CHIP_ID;

    /* 溫度校正資料 */
    s->regs[0x88] = 0x70; s->regs[0x89] = 0x6B; /* T1 */
    s->regs[0x8A] = 0x43; s->regs[0x8B] = 0x67; /* T2 */
    s->regs[0x8C] = 0x18; s->regs[0x8D] = 0x00; /* T3 */

    /* 溫度原始資料：約 25°C */
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

    s->reg_addr  = 0;
    s->reading   = false;
    s->byte_count = 0;
}

static uint32_t bme280_spi_transfer(SSIPeripheral *dev, uint32_t val)
{
    BME280SPIState *s = BME280_SPI(dev);

    if (s->byte_count == 0) {
        /* 第一個 byte：控制 byte */
        s->reading  = (val & 0x80) != 0;  /* bit7=1 讀，bit7=0 寫 */
        s->reg_addr = (val & 0x7F) | (s->reading ? 0x80 : 0x00);          /* bit6~0 = 暫存器位址 */
        s->byte_count++;
        printf("[BME280 SPI] %s reg=0x%02x\n",
               s->reading ? "READ" : "WRITE", s->reg_addr);
        fflush(stdout);
        return 0;
    } else {
        /* 之後的 byte：資料 */
        uint32_t ret = 0;
        if (s->reading) {
            ret = s->regs[s->reg_addr];
            printf("[BME280 SPI] READ  reg=0x%02x val=0x%02x\n",
                   s->reg_addr, ret);
        } else {
            if (val == BME280_SOFT_RESET_VAL &&
                s->reg_addr == BME280_REG_RESET) {
                bme280_spi_reset(s);
            } else {
                s->regs[s->reg_addr] = val;
            }
            printf("[BME280 SPI] WRITE reg=0x%02x val=0x%02x\n",
                   s->reg_addr, (uint8_t)val);
        }
        fflush(stdout);
        s->reg_addr++;
        s->byte_count++;
        return ret;
    }
}

static int bme280_spi_set_cs(SSIPeripheral *dev, bool select)
{
    BME280SPIState *s = BME280_SPI(dev);
    if (!select) {
        /* CS 拉高，傳輸結束，重置狀態機 */
        s->byte_count = 0;
        s->reading    = false;
    }
    return 0;
}

static void bme280_spi_realize(SSIPeripheral *dev, Error **errp)
{
    BME280SPIState *s = BME280_SPI(dev);
    bme280_spi_reset(s);
    printf("[BME280 SPI] realize called!\n");
    fflush(stdout);
}

static void bme280_spi_class_init(ObjectClass *klass, const void *data)
{
    SSIPeripheralClass *k = SSI_PERIPHERAL_CLASS(klass);

    k->realize      = bme280_spi_realize;
    k->transfer     = bme280_spi_transfer;
    k->set_cs       = bme280_spi_set_cs;
    k->cs_polarity  = SSI_CS_LOW;
}

static const TypeInfo bme280_spi_info = {
    .name          = TYPE_BME280_SPI,
    .parent        = TYPE_SSI_PERIPHERAL,
    .instance_size = sizeof(BME280SPIState),
    .class_init    = bme280_spi_class_init,
};

static void bme280_spi_register_types(void)
{
    type_register_static(&bme280_spi_info);
}
type_init(bme280_spi_register_types)

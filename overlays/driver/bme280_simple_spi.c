#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/sysfs.h>
#include <linux/device.h>

/* BME280 暫存器 */
#define BME280_REG_ID        0xD0
#define BME280_REG_TEMP_MSB  0xFA
#define BME280_REG_TEMP_LSB  0xFB
#define BME280_REG_TEMP_XLSB 0xFC
#define BME280_CHIP_ID       0x60

/* SPI 讀取：bit7=1 表示讀取 */
#define BME280_SPI_READ(reg)  ((reg) | 0x80)
#define BME280_SPI_WRITE(reg) ((reg) & 0x7F)

static int bme280_spi_read_reg(struct spi_device *spi, u8 reg, u8 *val)
{
    u8 tx = BME280_SPI_READ(reg);
    u8 rx = 0;
    struct spi_transfer t[2] = {
        { .tx_buf = &tx, .len = 1 },
        { .rx_buf = &rx, .len = 1 },
    };
    struct spi_message m;
    int ret;

    spi_message_init(&m);
    spi_message_add_tail(&t[0], &m);
    spi_message_add_tail(&t[1], &m);
    ret = spi_sync(spi, &m);
    if (ret)
        return ret;

    *val = rx;
    return 0;
}

/* --- sysfs show 函數 --- */
static ssize_t temperature_show(struct device *dev,
                                struct device_attribute *attr,
                                char *buf)
{
    struct spi_device *spi = to_spi_device(dev);
    u8 msb, lsb, xlsb;
    int raw, ret;

    ret = bme280_spi_read_reg(spi, BME280_REG_TEMP_MSB, &msb);
    if (ret) return ret;
    ret = bme280_spi_read_reg(spi, BME280_REG_TEMP_LSB, &lsb);
    if (ret) return ret;
    ret = bme280_spi_read_reg(spi, BME280_REG_TEMP_XLSB, &xlsb);
    if (ret) return ret;

    raw = (msb << 12) | (lsb << 4) | (xlsb >> 4);
    return sysfs_emit(buf, "%d\n", raw);
}

static ssize_t chip_id_show(struct device *dev,
                            struct device_attribute *attr,
                            char *buf)
{
    struct spi_device *spi = to_spi_device(dev);
    u8 chip_id;
    int ret;

    ret = bme280_spi_read_reg(spi, BME280_REG_ID, &chip_id);
    if (ret) return ret;

    return sysfs_emit(buf, "0x%02x\n", chip_id);
}

/* --- attribute 定義 --- */
static DEVICE_ATTR_RO(temperature);
static DEVICE_ATTR_RO(chip_id);

static struct attribute *bme280_spi_attrs[] = {
    &dev_attr_temperature.attr,
    &dev_attr_chip_id.attr,
    NULL
};

static const struct attribute_group bme280_spi_group = {
    .attrs = bme280_spi_attrs,
};

/* --- probe / remove --- */
static int bme280_simple_spi_probe(struct spi_device *spi)
{
    struct device *dev = &spi->dev;
    u8 chip_id;
    int ret;

    /* 確認裝置 */
    ret = bme280_spi_read_reg(spi, BME280_REG_ID, &chip_id);
    if (ret) {
        dev_err(dev, "failed to read chip_id\n");
        return ret;
    }

    if (chip_id != BME280_CHIP_ID) {
        dev_err(dev, "bad chip_id: 0x%02x\n", chip_id);
        return -ENODEV;
    }

    dev_info(dev, "BME280 SPI found, chip_id=0x%02x\n", chip_id);

    /* 建立 sysfs */
    ret = sysfs_create_group(&spi->dev.kobj, &bme280_spi_group);
    if (ret) {
        dev_err(dev, "failed to create sysfs\n");
        return ret;
    }

    return 0;
}

static void bme280_simple_spi_remove(struct spi_device *spi)
{
    sysfs_remove_group(&spi->dev.kobj, &bme280_spi_group);
    dev_info(&spi->dev, "BME280 SPI removed\n");
}

/* --- driver 定義 --- */
static const struct of_device_id bme280_simple_spi_of_match[] = {
    { .compatible = "vdtf,bme280-simple" },
    { }
};
MODULE_DEVICE_TABLE(of, bme280_simple_spi_of_match);

static const struct spi_device_id bme280_simple_spi_id[] = {
    { "bme280-simple", 0 },
    { }
};
MODULE_DEVICE_TABLE(spi, bme280_simple_spi_id);

static struct spi_driver bme280_simple_spi_driver = {
    .driver = {
        .name           = "bme280-simple-spi",
        .of_match_table = bme280_simple_spi_of_match,
    },
    .probe    = bme280_simple_spi_probe,
    .remove   = bme280_simple_spi_remove,
    .id_table = bme280_simple_spi_id,
};
module_spi_driver(bme280_simple_spi_driver);

MODULE_AUTHOR("VDTF");
MODULE_DESCRIPTION("Simple BME280 SPI driver");
MODULE_LICENSE("GPL");

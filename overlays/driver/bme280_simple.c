#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/sysfs.h>
#include <linux/device.h>

/* BME280 暫存器 */
#define BME280_REG_ID        0xD0
#define BME280_REG_TEMP_MSB  0xFA
#define BME280_REG_TEMP_LSB  0xFB
#define BME280_REG_TEMP_XLSB 0xFC
#define BME280_CHIP_ID       0x60

/* --- sysfs show 函數 --- */
static ssize_t temperature_show(struct device *dev,
                                struct device_attribute *attr,
                                char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    int msb, lsb, xlsb, raw;

    msb  = i2c_smbus_read_byte_data(client, BME280_REG_TEMP_MSB);
    lsb  = i2c_smbus_read_byte_data(client, BME280_REG_TEMP_LSB);
    xlsb = i2c_smbus_read_byte_data(client, BME280_REG_TEMP_XLSB);

    if (msb < 0 || lsb < 0 || xlsb < 0)
        return -EIO;

    raw = (msb << 12) | (lsb << 4) | (xlsb >> 4);

    return sysfs_emit(buf, "%d\n", raw);
}

static ssize_t chip_id_show(struct device *dev,
                            struct device_attribute *attr,
                            char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    int ret;

    ret = i2c_smbus_read_byte_data(client, BME280_REG_ID);
    if (ret < 0)
        return ret;

    return sysfs_emit(buf, "0x%02x\n", ret);
}

/* --- attribute 定義 --- */
static DEVICE_ATTR_RO(temperature);
static DEVICE_ATTR_RO(chip_id);

static struct attribute *bme280_attrs[] = {
    &dev_attr_temperature.attr,
    &dev_attr_chip_id.attr,
    NULL
};

static const struct attribute_group bme280_group = {
    .attrs = bme280_attrs,
};

/* --- probe / remove --- */
static int bme280_simple_probe(struct i2c_client *client)
{
    struct device *dev = &client->dev;
    int ret;
    u8 chip_id;

    /* 確認裝置 */
    ret = i2c_smbus_read_byte_data(client, BME280_REG_ID);
    if (ret < 0) {
        dev_err(dev, "failed to read chip_id\n");
        return ret;
    }
    chip_id = ret;

    if (chip_id != BME280_CHIP_ID) {
        dev_err(dev, "bad chip_id: 0x%02x\n", chip_id);
        return -ENODEV;
    }

    dev_info(dev, "BME280 found, chip_id=0x%02x\n", chip_id);

    /* 建立 sysfs */
    ret = sysfs_create_group(&client->dev.kobj, &bme280_group);
    if (ret) {
        dev_err(dev, "failed to create sysfs\n");
        return ret;
    }

    return 0;
}

static void bme280_simple_remove(struct i2c_client *client)
{
    sysfs_remove_group(&client->dev.kobj, &bme280_group);
    dev_info(&client->dev, "BME280 removed\n");
}

/* --- driver 定義 --- */
static const struct of_device_id bme280_simple_of_match[] = {
    { .compatible = "vdtf,bme280-simple" },
    { }
};
MODULE_DEVICE_TABLE(of, bme280_simple_of_match);

static const struct i2c_device_id bme280_simple_id[] = {
    { "bme280-simple", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, bme280_simple_id);

static struct i2c_driver bme280_simple_driver = {
    .driver = {
        .name           = "bme280-simple",
        .of_match_table = bme280_simple_of_match,
    },
    .probe   = bme280_simple_probe,
    .remove  = bme280_simple_remove,
    .id_table = bme280_simple_id,
};
module_i2c_driver(bme280_simple_driver);

MODULE_AUTHOR("VDTF");
MODULE_DESCRIPTION("Simple BME280 I2C driver");
MODULE_LICENSE("GPL");

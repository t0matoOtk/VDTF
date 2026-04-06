#!/bin/sh
modprobe i2c-bcm2835
modprobe spi-bcm2835
modprobe industrialio
modprobe kfifo_buf
modprobe industrialio-triggered-buffer
modprobe regmap-i2c
modprobe regmap-spi
modprobe bmp280
modprobe bmp280-i2c
modprobe bmp280-spi
echo "=== Done ==="
ls /sys/bus/iio/devices/

#!/bin/sh
modprobe i2c-bcm2835
modprobe spi-bcm2835
insmod /root/test/bme280_simple.ko
insmod /root/test/bme280_simple_spi.ko
echo "=== Done ==="
ls /sys/bus/i2c/devices/
ls /sys/bus/spi/devices/

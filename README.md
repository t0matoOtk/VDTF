# VDTF - Virtual Device Test Framework
[Note](https://hackmd.io/@UVDBF0TOR4uSkq00h3t9DQ/SkCfAgIjZx)
A Linux driver development and benchmarking platform built on QEMU raspi3b emulation.

## What it does

- Emulates BME280 sensor (I2C + SPI) in QEMU
- Implements custom kernel drivers for both interfaces
- Benchmarks latency, throughput, and CPU usage

## Quick Start
```bash
./scripts/setup.sh        # build kernel + rootfs
./scripts/setup_qemu.sh   # build QEMU with custom devices
./scripts/make_image.sh   # package image
./deploy/start_qemu.sh    # boot
```

## In guest
```bash
# upstream kernel driver
/root/test/load_kernel.sh
/root/test/bench kernel

# custom driver
/root/test/load_simple.sh
/root/test/bench simple
```

## Results

### I2C vs SPI (upstream driver)

| | I2C | SPI |
|--|-----|-----|
| avg | 264 us | 90 us |
| p99 | 832 us | 334 us |
| throughput | 3787 /sec | 11111 /sec |

SPI is ~3x faster due to no ACK overhead and full-duplex communication.

### Custom vs Upstream driver

| | I2C kernel | I2C simple | SPI kernel | SPI simple |
|--|-----------|------------|-----------|------------|
| avg | 264 us | 761 us | 90 us | 140 us |
| throughput | 3787 /sec | 1314 /sec | 11111 /sec | 7142 /sec |

Upstream driver is faster due to regmap caching and IIO framework optimizations.

## QEMU Limitations

Timing reflects software overhead only. On real hardware, SPI advantage would be more pronounced (100kHz vs 10MHz clock).

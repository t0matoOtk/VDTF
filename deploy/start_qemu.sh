#!/bin/bash
# Path resolution for cross-directory execution
DEPLOY_DIR=$(cd "$(dirname "$0")"; pwd)
PROJECT_ROOT=$(cd "$DEPLOY_DIR/.."; pwd)

KERNEL="$PROJECT_ROOT/build/linux/arch/arm64/boot/Image"
DTB="$PROJECT_ROOT/build/linux/arch/arm64/boot/dts/broadcom/bcm2710-rpi-3-b.dtb"
ROOTFS="$DEPLOY_DIR/rootfs.img"
NEW_QEMU="$PROJECT_ROOT/build/qemu/build/qemu-system-aarch64"

echo "🚀 Booting VDTF QEMU..."

$NEW_QEMU \
  -M raspi3b \
  -cpu cortex-a53 \
  -m 1G \
  -kernel "$KERNEL" \
  -dtb "$DTB" \
  -drive file="$ROOTFS",format=raw,if=sd \
  -append "rw console=tty1 root=/dev/mmcblk0 rootwait devtmpfs.mount=1" \
  -device usb-kbd \
  -device usb-tablet \
  -serial stdio

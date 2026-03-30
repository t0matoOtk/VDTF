#!/bin/bash
set -e

# --- Configuration ---
ARCH=arm64
CROSS_COMPILE=aarch64-linux-gnu-
BUILD_DIR="$(pwd)/build"
CONFIGS_DIR="$(pwd)/configs"
OVERLAY_DIR="$(pwd)/overlays/rootfs" 
ROOTFS_DIR="$BUILD_DIR/rootfs"

mkdir -p "$BUILD_DIR" "$CONFIGS_DIR"

# --- 1. BusyBox ---
echo "==== Processing BusyBox ===="
[ ! -d "$BUILD_DIR/busybox" ] && git clone --depth 1 https://git.busybox.net/busybox "$BUILD_DIR/busybox"

cd "$BUILD_DIR/busybox"
if [ -f "$CONFIGS_DIR/busybox.config" ]; then
    cp "$CONFIGS_DIR/busybox.config" .config
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE oldconfig
else
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig
fi

make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE prepare
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE -j$(nproc) install

# --- 2. Linux Kernel ---
echo "==== Processing Linux Kernel ===="
[ ! -d "$BUILD_DIR/linux" ] && git clone --depth 1 https://github.com/raspberrypi/linux.git "$BUILD_DIR/linux"

cd "$BUILD_DIR/linux"
if [ -f "$CONFIGS_DIR/linux.config" ]; then
    cp "$CONFIGS_DIR/linux.config" .config
else
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE bcm2711_defconfig
fi

make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE -j$(nproc) Image dtbs

# --- 3. RootFS Integration ---
echo "==== Constructing RootFS ===="
rm -rf "$ROOTFS_DIR"
mkdir -p "$ROOTFS_DIR"
cp -ra "$BUILD_DIR/busybox/_install/"* "$ROOTFS_DIR/"

# Create FHS directories
cd "$ROOTFS_DIR"
mkdir -p dev etc/init.d proc sys tmp mnt root 

# Apply Overlays
if [ -d "$OVERLAY_DIR" ]; then
    echo "🚀 Applying custom overlays..."
    cp -ra "$OVERLAY_DIR/"* "$ROOTFS_DIR/"
else
    echo "❌ Error: Overlay directory missing at $OVERLAY_DIR"
    exit 1
fi

echo "✅ Setup complete!"

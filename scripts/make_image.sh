#!/bin/bash
set -e

# --- Configuration ---
IMG_NAME="deploy/rootfs.img"
ROOTFS_DIR="build/rootfs"
IMG_SIZE="128M"

echo "==== Packaging RootFS Image ===="

if [ ! -d "$ROOTFS_DIR" ]; then
    echo "❌ Error: $ROOTFS_DIR not found!"
    exit 1
fi

mkdir -p deploy
truncate -s $IMG_SIZE $IMG_NAME
mkfs.ext4 -F $IMG_NAME

# Mount and Sync
echo "Writing files to image (Sudo required)..."
TMP_MNT=$(mktemp -d)
sudo mount $IMG_NAME $TMP_MNT
sudo cp -ra "$ROOTFS_DIR/"* "$TMP_MNT/"
sudo umount $TMP_MNT
rmdir $TMP_MNT

echo "✅ Image ready: $IMG_NAME"

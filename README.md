# Quick Start

## 1. Environment Setup

Ensure your Linux host (Ubuntu 22.04 or later recommended) has the required cross-compilation tools installed:

```bash
sudo apt update
sudo apt install git bc bison flex libssl-dev make libc6-dev \
libncurses5-dev crossbuild-essential-arm64 qemu-system-arm
```

## 2. Initialize and Build

Run the setup script. It will download the Linux kernel and BusyBox source code, apply the configurations in `configs/`, and complete the build process.

```bash
./scripts/setup.sh
```

## 3. Build RootFS Image

Package the generated root filesystem into an ext4 disk image:

```bash
./scripts/make_image.sh
```

## 4. Launch QEMU Emulator

```bash
./deploy/start_qemu.sh
```

---

## Project Structure

* **configs/**
  Contains custom configurations for the Linux kernel (`linux.config`) and BusyBox (`busybox.config`).

* **rootfs_overlay/**
  Stores custom system files, such as `etc/inittab` and `etc/init.d/rcS`.

* **scripts/**
  Includes scripts for environment setup, automated builds, and image generation.

* **deploy/**
  Contains launch scripts and the final generated images.
  
* **build/** 
  (Generated) Source code and intermediate RootFS directory.
  
* **overlays/**
  Custom system files (e.g., etc/inittab, rcS) to be merged into RootFS.


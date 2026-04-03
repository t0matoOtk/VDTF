#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "$0")"; pwd)"
PROJECT_ROOT_DIR="$(cd "$SCRIPT_DIR/.."; pwd)"
REPO_URL="https://github.com/qemu/qemu.git"
SOURCE_DIR="$PROJECT_ROOT_DIR/build/qemu"
OVERLAY_DIR="$PROJECT_ROOT_DIR/overlays/qemu"

if [ ! -d "$SOURCE_DIR" ]; then
    echo "--- 正在從 GitHub 下載 QEMU 原始碼 ---"
    git clone --depth 1 $REPO_URL $SOURCE_DIR
else
    echo "--- 原始碼已存在，跳過下載 ---"
fi

rsync -av $OVERLAY_DIR/ $SOURCE_DIR/

cd $SOURCE_DIR
mkdir -p build
cd build

if [ ! -f "build.ninja" ]; then
    ../configure --target-list=aarch64-softmmu
else
    /usr/local/bin/meson setup --reconfigure ..
fi

ninja -j$(nproc)
echo "--- 編譯完成！ ---"



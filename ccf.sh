#!/usr/bin/env bash
#
# clean + build + st-flash - STM32F407 "rx" projesi
#
set -euo pipefail

# ----------------------------------------------------------------------
# AYARLAR - kendi kurulumuna gore BIR KERE duzenle
# ----------------------------------------------------------------------
BUILD_DIR="build"
TARGET_NAME="rx"                            # CMakeLists.txt'deki project() adi
TOOLCHAIN_FILE="cmake/gcc-arm-none-eabi.cmake"
FLASH_ADDR="0x08000000"

# CMakePresets.json kullaniyorsan USE_PRESET=1 yap ve PRESET_NAME'i
# kendi preset adinla degistir (cmake --list-presets ile gor).
USE_PRESET=0
PRESET_NAME="Debug"
# ----------------------------------------------------------------------

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_ROOT"

echo "==> [1/4] Clean"
rm -rf "$BUILD_DIR"

echo "==> [2/4] Configure"
if [ "$USE_PRESET" -eq 1 ]; then
    cmake --preset "$PRESET_NAME"
else
    cmake -B "$BUILD_DIR" -G "Unix Makefiles" \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
        -DCMAKE_BUILD_TYPE=Debug
fi

echo "==> [3/4] Build"
cmake --build "$BUILD_DIR" -j"$(nproc)"

ELF_FILE="${BUILD_DIR}/${TARGET_NAME}.elf"
BIN_FILE="${BUILD_DIR}/${TARGET_NAME}.bin"

if [ ! -f "$ELF_FILE" ]; then
    echo "Hata: $ELF_FILE bulunamadi. Build basarisiz olmus olabilir."
    exit 1
fi

# .bin dosyasini elf'ten garanti olarak uret
arm-none-eabi-objcopy -O binary "$ELF_FILE" "$BIN_FILE"

echo "==> [4/4] Flash (st-flash)"
sudo st-flash write "$BIN_FILE" "$FLASH_ADDR"

echo "==> Tamamlandi."
#!/bin/sh
set -e

TARGET=${1:-simulator}

case "$TARGET" in
  simulator)
    [ -d build ] || cmake --fresh -S simulator -B build -G Ninja
    cmake --build build
    ./build/simulator
    ;;
  esp32)
    idf.py -C esp32 flash monitor
    ;;
  *)
    echo "Usage: $0 [simulator|esp32]"
    exit 1
    ;;
esac

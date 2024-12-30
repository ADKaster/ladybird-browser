#!/bin/sh
set -e

GN_VERSION=${GN_VERSION:-master}
BUILD_DIR="/tmp/gn_build"

cd /tmp
git clone https://gn.googlesource.com/gn
cd gn
git fetch origin
git checkout "$GN_VERSION"

./build/gen.py --out-path="$BUILD_DIR" --allow-warnings
ninja -C "$BUILD_DIR"

mkdir -p /usr/bin
cp "$BUILD_DIR/gn" /usr/local/bin

# Clean up
cd /tmp
rm -rf gn
rm -rf "$BUILD_DIR"

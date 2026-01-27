#!/bin/sh
set -e

METAL_TOOLCHAIN="/private/var/run/com.apple.security.cryptexd/mnt/com.apple.MobileAsset.MetalToolchain-v17.1.324.0.uBC9Rf/Metal.xctoolchain"
XCODE_TOOLCHAIN="/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain"
METAL_SRC="$METAL_TOOLCHAIN/usr/bin/metal"
METALLIB_SRC="$METAL_TOOLCHAIN/usr/bin/metallib"
METAL_DST="$XCODE_TOOLCHAIN/usr/bin/metal"
METALLIB_DST="$XCODE_TOOLCHAIN/usr/bin/metallib"

if [ "$1" = "--restore" ]; then
  if [ -e "$METAL_DST.stub" ]; then
    sudo mv "$METAL_DST.stub" "$METAL_DST"
  fi
  sudo rm -f "$METALLIB_DST"
  echo "Restored Xcode metal stub."
  exit 0
fi

if [ ! -x "$METAL_SRC" ]; then
  echo "Metal toolchain not found at: $METAL_TOOLCHAIN"
  echo "Install it in Xcode > Settings > Components first."
  exit 1
fi

if [ -e "$METAL_DST" ] && [ ! -L "$METAL_DST" ]; then
  sudo mv "$METAL_DST" "$METAL_DST.stub"
fi

sudo ln -sfn "$METAL_SRC" "$METAL_DST"
sudo ln -sfn "$METALLIB_SRC" "$METALLIB_DST"

echo "Linked metal/metallib into XcodeDefault toolchain."
echo "Verify:"
echo "  $METAL_DST -v"
echo "  $METALLIB_DST -v"

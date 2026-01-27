#!/bin/sh
set -e

TOOLCHAIN_SRC="/private/var/run/com.apple.security.cryptexd/mnt/com.apple.MobileAsset.MetalToolchain-v17.1.324.0.uBC9Rf/Metal.xctoolchain"
TOOLCHAIN_DST="/Applications/Xcode.app/Contents/Developer/Toolchains/Metal.xctoolchain"

if [ ! -d "$TOOLCHAIN_SRC" ]; then
  echo "Metal toolchain not found at: $TOOLCHAIN_SRC"
  echo "Install it in Xcode > Settings > Components first."
  exit 1
fi

echo "Linking Metal toolchain into Xcode Toolchains..."
sudo ln -sfn "$TOOLCHAIN_SRC" "$TOOLCHAIN_DST"

echo "Done."
echo "Verify:"
echo "  ls -l \"$TOOLCHAIN_DST\""
echo "  xcrun metal -v"

# Project Structure - Generator-Friendly Layout

**Version**: 1.0.0
**Last Updated**: 2026-01-25
**Status**: Active

> このドキュメントは oflike-metal プロジェクトジェネレータが生成する標準的なフォルダ構造を定義します。

---

## Overview

oflike-metal のプロジェクトジェネレータは、標準化されたフォルダ構造を生成します。この構造は以下の要件を満たします:

- ✅ **統一性**: すべての生成プロジェクトが同じ構造を持つ
- ✅ **予測可能性**: ユーザーがファイルの場所を直感的に理解できる
- ✅ **拡張性**: addons やリソースを追加しやすい
- ✅ **ビルドツール対応**: Xcode, CMake 両方に対応
- ✅ **openFrameworks 互換**: oF ユーザーが違和感なく使える

---

## Standard Generated Project Layout

### Complete Folder Structure

```
myProject/                          # プロジェクトルート
├── src/                            # アプリケーションソースコード
│   ├── MyApp.h                     # アプリケーションヘッダー
│   ├── MyApp.cpp                   # アプリケーション実装 (C++)
│   ├── main.mm                     # エントリーポイント (ofMain 使用時)
│   └── App.swift                   # エントリーポイント (SwiftUI 使用時)
│
├── addons/                         # プロジェクト固有の addons
│   ├── ofxMyAddon/                 # カスタム addon 例
│   │   ├── src/
│   │   │   ├── ofxMyAddon.h
│   │   │   └── ofxMyAddon.cpp
│   │   ├── libs/                   # addon の外部ライブラリ
│   │   └── README.md
│   └── README.md                   # addons フォルダの説明
│
├── data/                           # ランタイムデータ
│   ├── images/                     # 画像ファイル
│   │   └── .gitkeep
│   ├── fonts/                      # フォントファイル
│   │   └── .gitkeep
│   ├── sounds/                     # オーディオファイル
│   │   └── .gitkeep
│   ├── videos/                     # ビデオファイル
│   │   └── .gitkeep
│   ├── models/                     # 3Dモデル (.obj, .gltf, .usdz)
│   │   └── .gitkeep
│   ├── shaders/                    # カスタムシェーダー (.metal)
│   │   └── .gitkeep
│   └── README.md                   # data フォルダの説明
│
├── resources/                      # ビルド時にバンドルされるリソース
│   ├── Assets.xcassets/            # Xcode アセットカタログ
│   │   └── AppIcon.appiconset/
│   ├── Info.plist                  # macOS アプリケーション情報
│   └── Entitlements.plist          # サンドボックス/機能設定
│
├── shaders/                        # プロジェクト固有のシェーダー
│   ├── Default.metal               # デフォルトシェーダー
│   └── README.md
│
├── build/                          # ビルド生成物 (gitignore)
│   ├── Debug/
│   ├── Release/
│   └── .gitignore
│
├── tools/                          # ビルド/開発ツール
│   ├── scripts/                    # ビルドスクリプト
│   │   ├── build.sh
│   │   └── package.sh
│   └── README.md
│
├── docs/                           # プロジェクトドキュメント
│   ├── README.md                   # プロジェクト説明
│   └── ARCHITECTURE.md             # アーキテクチャドキュメント (オプション)
│
├── tests/                          # ユニットテスト (オプション)
│   ├── TestApp.h
│   └── TestApp.cpp
│
├── CMakeLists.txt                  # CMake ビルド設定
├── project.yml                     # XcodeGen プロジェクト定義
├── .gitignore                      # Git 除外設定
├── README.md                       # プロジェクト README
└── LICENSE                         # ライセンス (オプション)
```

---

## Folder Descriptions

### `/src/` - Application Source Code

**目的**: アプリケーションのメインソースコード

**標準ファイル**:
- `MyApp.h` - アプリケーションクラス定義
- `MyApp.cpp` - アプリケーション実装
- `main.mm` - ofMain エントリーポイント (レガシー)
- `App.swift` - SwiftUI エントリーポイント (デフォルト)

**ユーザー追加ファイル**:
- 追加のクラス、ヘルパー、ユーティリティ
- プロジェクト固有のロジック

**例**:
```
src/
├── MyApp.h
├── MyApp.cpp
├── App.swift              # SwiftUI Entry (デフォルト)
├── ParticleSystem.h       # ユーザー追加
├── ParticleSystem.cpp
└── Utils.h
```

---

### `/addons/` - Project-Specific Addons

**目的**: プロジェクト固有の addon やカスタム addon

**標準構造**:
```
addons/
├── ofxMyAddon/
│   ├── src/
│   │   ├── ofxMyAddon.h
│   │   └── ofxMyAddon.cpp
│   ├── libs/              # サードパーティライブラリ
│   ├── examples/          # addon の使用例
│   └── README.md
└── README.md
```

**使用方法**:
- Core Addons は oflike-metal に含まれる (`src/addons/core/`)
- Apple Native Addons も oflike-metal に含まれる (`src/addons/native/`)
- このフォルダはプロジェクト固有またはカスタム addon 用

**ビルド設定**:
- `CMakeLists.txt` で `add_subdirectory(addons/ofxMyAddon)` を追加
- `project.yml` で `sources: [addons/ofxMyAddon/src]` を追加

---

### `/data/` - Runtime Data

**目的**: ランタイムに読み込むデータファイル

**サブフォルダ**:
- `images/` - PNG, JPEG, TIFF, EXR, HDR 画像
- `fonts/` - TrueType, OpenType フォント
- `sounds/` - WAV, MP3, AAC オーディオ
- `videos/` - MP4, MOV ビデオ
- `models/` - OBJ, glTF, USDZ 3Dモデル
- `shaders/` - カスタム Metal シェーダー (.metal)

**ランタイムアクセス**:
```cpp
// data/ フォルダはアプリケーションバンドル内にコピーされる
ofImage img;
img.load("images/photo.jpg");  // data/images/photo.jpg から読み込み

ofTrueTypeFont font;
font.load("fonts/arial.ttf", 24);
```

**ビルド設定**:
- Xcode: "Copy Bundle Resources" に追加
- CMake: `install(DIRECTORY data/ DESTINATION ${CMAKE_BINARY_DIR}/data)`

---

### `/resources/` - Build-Time Resources

**目的**: ビルド時にアプリケーションバンドルに含まれるリソース

**標準ファイル**:
- `Assets.xcassets/` - Xcode アセットカタログ (アイコン、画像)
- `Info.plist` - アプリケーション情報
- `Entitlements.plist` - サンドボックス、機能設定

**Info.plist 例**:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>
    <string>MyApp</string>
    <key>CFBundleIdentifier</key>
    <string>com.example.myapp</string>
    <key>CFBundleVersion</key>
    <string>1.0.0</string>
    <key>LSMinimumSystemVersion</key>
    <string>13.0</string>
</dict>
</plist>
```

---

### `/shaders/` - Custom Shaders

**目的**: プロジェクト固有の Metal シェーダー

**標準ファイル**:
- `Default.metal` - デフォルトシェーダー (テンプレート)

**カスタムシェーダー例**:
```
shaders/
├── Default.metal          # 基本シェーダー
├── Blur.metal             # ブラーシェーダー
└── Particles.metal        # パーティクルシェーダー
```

**使用方法**:
```cpp
// シェーダーはビルド時にコンパイルされる
id<MTLLibrary> library = [device newDefaultLibrary];
id<MTLFunction> vertexFunc = [library newFunctionWithName:@"vertexShader"];
```

---

### `/build/` - Build Artifacts

**目的**: ビルド生成物の出力先

**サブフォルダ**:
- `Debug/` - デバッグビルド
- `Release/` - リリースビルド

**gitignore**: このフォルダ全体を `.gitignore` に追加

---

### `/tools/` - Build and Development Tools

**目的**: ビルドスクリプト、開発ツール

**標準スクリプト**:
```
tools/
├── scripts/
│   ├── build.sh           # ビルドスクリプト
│   ├── package.sh         # パッケージング
│   └── clean.sh           # クリーンアップ
└── README.md
```

**build.sh 例**:
```bash
#!/bin/bash
# Build script for myProject
cd "$(dirname "$0")/../build"
cmake .. -G Xcode
cmake --build . --config Release
```

---

### `/docs/` - Project Documentation

**目的**: プロジェクト固有のドキュメント

**標準ファイル**:
- `README.md` - プロジェクト説明
- `ARCHITECTURE.md` - アーキテクチャドキュメント (オプション)

---

### `/tests/` - Unit Tests (Optional)

**目的**: ユニットテスト

**例**:
```cpp
// tests/TestApp.cpp
#include "oflike/ofMain.h"

void testParticleSystem() {
    ParticleSystem ps;
    ps.addParticle(ofVec3f(0, 0, 0));
    assert(ps.size() == 1);
}
```

---

## Generator Behavior

### Project Generator Commands

**基本生成**:
```bash
oflike-gen new myProject
```

生成されるファイル:
- `src/MyApp.h`, `src/MyApp.cpp`
- `src/App.swift` (SwiftUI Entry)
- `CMakeLists.txt`, `project.yml`
- `data/`, `resources/`, `shaders/` (空のフォルダ)
- `.gitignore`, `README.md`

**Addon を含む生成**:
```bash
oflike-gen new myProject --addons ofxOsc ofxGui
```

追加される内容:
- `CMakeLists.txt` に addon リンク追加
- `project.yml` に addon ソース追加

**ofMain Entry で生成**:
```bash
oflike-gen new myProject --entry ofMain
```

生成されるファイル:
- `src/main.mm` (ofMain Entry)
- SwiftUI ファイルは生成されない

---

## CMakeLists.txt Template

```cmake
cmake_minimum_required(VERSION 3.20)
project(myProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# oflike-metal framework
find_package(oflike-metal REQUIRED)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.mm")
file(GLOB_RECURSE HEADERS "src/*.h")

# Executable
add_executable(myProject ${SOURCES} ${HEADERS})

# Link oflike-metal
target_link_libraries(myProject
    oflike-metal::oflike-metal
)

# Include directories
target_include_directories(myProject PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Copy data folder
install(DIRECTORY data/ DESTINATION ${CMAKE_BINARY_DIR}/data)

# Addons (if any)
# add_subdirectory(addons/ofxMyAddon)
# target_link_libraries(myProject ofxMyAddon)
```

---

## project.yml Template (XcodeGen)

```yaml
name: myProject
options:
  bundleIdPrefix: com.example
  deploymentTarget:
    macOS: "13.0"

targets:
  myProject:
    type: application
    platform: macOS

    sources:
      - src
      - data
      - resources
      - shaders

    settings:
      PRODUCT_BUNDLE_IDENTIFIER: com.example.myproject
      INFOPLIST_FILE: resources/Info.plist
      CODE_SIGN_ENTITLEMENTS: resources/Entitlements.plist
      SWIFT_VERSION: "5.9"
      CLANG_CXX_LANGUAGE_STANDARD: "c++20"

    dependencies:
      - framework: oflike-metal.framework
        embed: true

      # Apple Frameworks
      - framework: Metal
      - framework: MetalKit
      - framework: SwiftUI
      - framework: Foundation
```

---

## .gitignore Template

```gitignore
# Build artifacts
build/
*.xcodeproj
*.xcworkspace

# Xcode
*.pbxuser
*.mode1v3
*.mode2v3
*.perspectivev3
xcuserdata/
*.moved-aside
DerivedData/
*.hmap
*.ipa

# macOS
.DS_Store
.AppleDouble
.LSOverride

# CMake
CMakeCache.txt
CMakeFiles/
cmake_install.cmake

# IDE
.vscode/
.idea/
```

---

## Migration from openFrameworks

openFrameworks プロジェクト構造との対応:

| openFrameworks | oflike-metal | 備考 |
|----------------|--------------|------|
| `src/` | `src/` | 同じ |
| `bin/data/` | `data/` | パスが変更 |
| `addons/` (local) | `addons/` | 同じ |
| `Makefile` | `CMakeLists.txt` | ビルドシステム変更 |
| N/A | `resources/` | macOS 固有 |
| N/A | `shaders/` | Metal シェーダー |

**データパスの変更**:
```cpp
// openFrameworks
ofImage img;
img.load("myImage.png");  // bin/data/myImage.png

// oflike-metal
ofImage img;
img.load("images/myImage.png");  // data/images/myImage.png
```

---

## Best Practices

### 1. データの整理

✅ **DO**: ファイルタイプごとにサブフォルダに整理
```
data/
├── images/
│   ├── textures/
│   └── sprites/
├── fonts/
└── sounds/
    ├── music/
    └── sfx/
```

❌ **DON'T**: すべてのファイルを data/ 直下に配置

### 2. Addon の管理

✅ **DO**: カスタム addon は addons/ に配置
```
addons/
├── ofxMyCustomAddon/
└── ofxThirdPartyAddon/
```

❌ **DON'T**: Core/Native addons を addons/ にコピー (すでに oflike-metal に含まれている)

### 3. リソースのバージョン管理

✅ **DO**: 小さなリソース (アイコン、設定ファイル) は Git に含める

❌ **DON'T**: 大きなリソース (ビデオ、大量の画像) は Git LFS を使用

### 4. ビルド設定

✅ **DO**: CMake と XcodeGen 両方をサポート

❌ **DON'T**: Xcode プロジェクトファイル (.xcodeproj) を Git にコミット

---

## Version History

| 日付 | バージョン | 変更内容 |
|------|-----------|---------|
| 2026-01-25 | 1.0.0 | 初版作成 (Phase 0.4) |

---

## See Also

- [ARCHITECTURE.md](ARCHITECTURE.md) - プロジェクトアーキテクチャ
- [MIGRATION.md](MIGRATION.md) - openFrameworks からの移行ガイド
- [IMPLEMENTATION.md](IMPLEMENTATION.md) - 実装ガイドライン

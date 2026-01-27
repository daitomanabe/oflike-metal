# Implementation Guidelines

**Version**: 1.0.2
**Last Updated**: 2026-01-25
**Status**: Active

> このドキュメントは oflike-metal の実装ガイドラインとプロジェクトジェネレータの仕様を定義します。

---

## Table of Contents

1. [Addon Linking Strategy](#addon-linking-strategy)
2. [Project Generator Specification](#project-generator-specification)
3. [Build System Integration](#build-system-integration)
4. [Coding Standards](#coding-standards)

---

## Addon Linking Strategy

### Overview

oflike-metal は3種類の addon を扱います:

1. **Core Addons** - oflike-metal に組み込み (ofxOsc, ofxGui, etc.)
2. **Apple Native Addons** - oflike-metal に組み込み (ofxSharp, ofxNeuralEngine, etc.)
3. **Custom Addons** - ユーザープロジェクト固有またはサードパーティ

プロジェクトジェネレータは、これらの addon をプロジェクトに統合するための**3つの戦略**を提供します。

---

### Strategy 1: Reference (推奨)

**概要**: oflike-metal フレームワークを参照し、addon を直接リンク。

#### 利点

✅ **ディスクスペース効率**: addon ファイルのコピー不要
✅ **常に最新**: oflike-metal を更新すれば addon も更新
✅ **シンプル**: ビルド設定が簡潔
✅ **推奨**: ほとんどのケースでこの戦略を使用

#### 使用ケース

- Core Addons を使用 (ofxOsc, ofxGui, etc.)
- Apple Native Addons を使用 (ofxSharp, ofxNeuralEngine, etc.)
- oflike-metal のバージョンに追従する標準的なプロジェクト

#### 実装

**CMakeLists.txt**:
```cmake
# oflike-metal をリンク (addon は自動的に利用可能)
find_package(oflike-metal REQUIRED)
target_link_libraries(myProject oflike-metal::oflike-metal)

# Core Addons はすでに含まれている
# #include <addons/core/ofxOsc/ofxOsc.h> で使用可能
```

**project.yml** (XcodeGen):
```yaml
targets:
  myProject:
    dependencies:
      - framework: oflike-metal.framework
        embed: true

    settings:
      HEADER_SEARCH_PATHS:
        - $(FRAMEWORK_SEARCH_PATHS)/oflike-metal.framework/Headers
```

**ユーザーコード**:
```cpp
// 直接 include (addon は oflike-metal に含まれる)
#include <addons/core/ofxOsc/ofxOsc.h>
#include <addons/native/ofxSharp/ofxSharp.h>

ofxOscSender sender;
ofxSharp sharp;
```

#### 制約

⚠️ **バージョン管理**: oflike-metal のバージョンに依存
⚠️ **カスタム addon**: この戦略では使用不可 (Copy または Symlink を使用)

---

### Strategy 2: Copy

**概要**: addon ファイルをプロジェクトフォルダにコピー。

#### 利点

✅ **完全独立**: oflike-metal から独立したプロジェクト
✅ **カスタマイズ可能**: addon を自由に編集可能
✅ **バージョン固定**: oflike-metal を更新しても影響なし
✅ **配布**: プロジェクト単体で配布可能

#### 使用ケース

- カスタム addon を使用
- addon を編集・カスタマイズする必要がある
- oflike-metal から完全に独立したプロジェクト
- プロジェクトを他者に配布する際に依存関係を含めたい

#### 実装

**プロジェクト構造**:
```
myProject/
├── addons/
│   ├── ofxMyCustomAddon/      # カスタム addon
│   │   ├── src/
│   │   ├── libs/
│   │   └── README.md
│   └── ofxThirdParty/         # サードパーティ addon (コピー)
│       ├── src/
│       └── README.md
├── src/
└── CMakeLists.txt
```

**Generator コマンド**:
```bash
# カスタム addon を copy モードで追加
oflike-gen add-addon ofxMyCustomAddon --mode copy --source /path/to/ofxMyCustomAddon

# addon ディレクトリを指定してコピー
oflike-gen add-addon ofxThirdParty --mode copy --source ~/addons/ofxThirdParty
```

**CMakeLists.txt**:
```cmake
# カスタム addon をビルドに含める
add_subdirectory(addons/ofxMyCustomAddon)
target_link_libraries(myProject ofxMyCustomAddon)

# addon のヘッダーパスを追加
target_include_directories(myProject PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/addons/ofxMyCustomAddon/src
)
```

**project.yml**:
```yaml
targets:
  myProject:
    sources:
      - addons/ofxMyCustomAddon/src
      - addons/ofxThirdParty/src

    settings:
      HEADER_SEARCH_PATHS:
        - addons/ofxMyCustomAddon/src
        - addons/ofxThirdParty/src
```

**ユーザーコード**:
```cpp
#include "ofxMyCustomAddon.h"
#include "ofxThirdParty.h"

ofxMyCustomAddon custom;
```

#### 制約

⚠️ **ディスクスペース**: addon ファイルが各プロジェクトにコピーされる
⚠️ **同期**: addon を更新する際は手動でコピーが必要
⚠️ **重複**: 複数プロジェクトで同じ addon を使用すると重複

---

### Strategy 3: Symlink

**概要**: addon ディレクトリへのシンボリックリンクを作成。

#### 利点

✅ **ディスクスペース効率**: 実ファイルは1つのみ
✅ **常に最新**: addon を更新すればすべてのプロジェクトに反映
✅ **管理しやすい**: 中央の addon ディレクトリで一元管理
✅ **開発効率**: addon の開発中に複数プロジェクトで即座にテスト可能

#### 使用ケース

- 複数プロジェクトで同じカスタム addon を共有
- addon を頻繁に更新する開発中
- ディスクスペースを節約したい
- 中央の addon リポジトリで管理

#### 実装

**プロジェクト構造**:
```
myProject/
├── addons/
│   ├── ofxMyCustomAddon -> ~/addons/ofxMyCustomAddon  # シンボリックリンク
│   └── ofxThirdParty -> ~/addons/ofxThirdParty        # シンボリックリンク
├── src/
└── CMakeLists.txt
```

**Generator コマンド**:
```bash
# シンボリックリンクで addon を追加
oflike-gen add-addon ofxMyCustomAddon --mode symlink --source ~/addons/ofxMyCustomAddon

# 複数プロジェクトで同じ addon を共有
cd ../anotherProject
oflike-gen add-addon ofxMyCustomAddon --mode symlink --source ~/addons/ofxMyCustomAddon
```

**手動でシンボリックリンクを作成**:
```bash
cd myProject/addons
ln -s ~/addons/ofxMyCustomAddon ofxMyCustomAddon
ln -s ~/addons/ofxThirdParty ofxThirdParty
```

**CMakeLists.txt**:
```cmake
# シンボリックリンクされた addon (Copy と同じ設定)
add_subdirectory(addons/ofxMyCustomAddon)
target_link_libraries(myProject ofxMyCustomAddon)
```

**project.yml**:
```yaml
targets:
  myProject:
    sources:
      - addons/ofxMyCustomAddon/src

    settings:
      HEADER_SEARCH_PATHS:
        - addons/ofxMyCustomAddon/src
```

#### 制約

⚠️ **プラットフォーム依存**: Windows ではシンボリックリンクに管理者権限が必要な場合あり
⚠️ **移植性**: プロジェクトを配布する際にシンボリックリンクが壊れる
⚠️ **意図しない変更**: addon を編集すると全プロジェクトに影響
⚠️ **Git 管理**: シンボリックリンクをコミットする際の注意が必要

---

## Strategy Comparison

| 項目 | Reference | Copy | Symlink |
|------|-----------|------|---------|
| **ディスクスペース** | 最小 | 大 | 最小 |
| **独立性** | 低 (oflike-metal依存) | 高 | 中 |
| **同期** | 自動 | 手動 | 自動 |
| **カスタマイズ** | 不可 | 可 | 可 |
| **配布** | フレームワーク必須 | 容易 | 難 |
| **推奨用途** | Core/Native Addons | カスタム addon | 開発中の addon |
| **ビルド設定** | シンプル | やや複雑 | やや複雑 |

---

## Project Generator Specification

### Overview

**Full CLI reference**: [tools/project_generator/README.md](../tools/project_generator/README.md)

このセクションは CLI 仕様の概要を提供します。詳細は [tools/project_generator/README.md](../tools/project_generator/README.md) を参照してください。

---

### Command: `oflike-gen new`

新規プロジェクトを生成します。

#### Syntax

```bash
oflike-gen new <project-name> [options]
```

#### Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `--addons <LIST>` | comma-separated | 初期 addon リスト | なし |
| `--addon-mode <MODE>` | `reference \| copy \| symlink` | Addon 統合方法 | `reference` |
| `--path <DIR>` | path | プロジェクト作成先 | `<oflike-root>/apps/<project-name>` |
| `--template <NAME>` | `basic \| swiftui \| metal \| 3d` | テンプレート | `basic` |
| `--bundle-id <ID>` | string | macOS bundle identifier | `com.example.<project>` |
| `--author <NAME>` | string | Project author name | Git config user.name |
| `--no-git` | flag | Skip git initialization | false |

※ `--path` を省略した場合は `<oflike-root>/apps` を自動検出して使用します。`~/.oflike-gen.toml` の `paths.oflike_metal_root` も利用可能。  
※ `project.yml` 生成後に `xcodegen generate` を自動実行して `.xcodeproj` を作成します。`project.yml` を変更した場合は手動で `xcodegen generate` を再実行してください。

#### Examples

**基本的なプロジェクト (SwiftUI Entry)**:
```bash
oflike-gen new myProject
```

生成される構造:
```
myProject/
├── src/
│   ├── MyApp.h
│   ├── MyApp.cpp
│   ├── App.swift         # SwiftUI Entry
│   ├── MetalView.swift   # MTKView + bridge
│   └── PerformanceMonitor.swift # FPS/stats overlay
├── data/
├── resources/
│   ├── Info.plist
│   └── Assets.xcassets/
├── CMakeLists.txt
├── project.yml
├── .gitignore
└── README.md
```


**Core Addons を含む**:
```bash
oflike-gen new myProject --addons ofxOsc,ofxGui
```

`CMakeLists.txt` に自動的に追加:
```cmake
# Core Addons (Reference モード)
# ofxOsc, ofxGui は oflike-metal に含まれる
```

**3D Template**:
```bash
oflike-gen new modelViewer --template 3d --addons ofxSharp
```

---

### Command: `oflike-gen add-addon`

既存プロジェクトに addon を追加します。

#### Syntax

```bash
oflike-gen add-addon <addon-name> [options]
```

#### Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `--mode <MODE>` | `reference \| copy \| symlink` | 統合方法 | `reference` |
| `--source <PATH>` | path | Addon ソースパス | auto-detect |
| `--project <PATH>` | path | プロジェクトパス | `.` |
| `--update-build` | flag | Update build files | true |

#### Examples

**Core Addon を追加 (Reference)**:
```bash
cd myProject
oflike-gen add-addon ofxOsc
```

**カスタム addon を Copy**:
```bash
oflike-gen add-addon ofxMyAddon --mode copy --source ~/addons/ofxMyAddon
```

**カスタム addon を Symlink**:
```bash
oflike-gen add-addon ofxMyAddon --mode symlink --source ~/addons/ofxMyAddon
```

---

### Command: `oflike-gen remove-addon`

プロジェクトから addon を削除します。

#### Syntax

```bash
oflike-gen remove-addon <addon-name> [options]
```

#### Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `--project <PATH>` | path | プロジェクトパス | `.` |
| `--keep-files` | flag | ファイルを残す (ビルド設定のみ削除) | false |

#### Examples

**Addon を完全削除**:
```bash
cd myProject
oflike-gen remove-addon ofxMyAddon
```

**ビルド設定のみ削除 (ファイルは残す)**:
```bash
oflike-gen remove-addon ofxMyAddon --keep-files
```

---

### Command: `oflike-gen list-addons`

利用可能な addon またはプロジェクト内の addon を一覧表示します。

#### Syntax

```bash
oflike-gen list-addons [options]
```

#### Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `--available` | flag | 利用可能な全 addon を表示 | false |
| `--project <PATH>` | path | プロジェクト内の addon を表示 | `.` |

#### Example

```bash
# 利用可能な addon 一覧
oflike-gen list-addons --available

# プロジェクト内の addon 一覧
cd myProject
oflike-gen list-addons
```

---

### Input Schema

プロジェクト設定は `oflike-gen.toml` または `project.json` で定義できます。

#### TOML Format (`oflike-gen.toml`)

```toml
[project]
name = "myProject"
version = "1.0.0"
author = "John Doe"
bundle_id = "com.example.myproject"

[entry]

[addons]
core = ["ofxOsc", "ofxGui", "ofxSharp"]
custom = [
    { name = "ofxMyAddon", mode = "copy", source = "~/addons/ofxMyAddon" }
]

[build]
cmake = true
xcodegen = true
min_macos = "13.0"
swift_version = "5.9"
cpp_standard = "c++20"

[paths]
src = "src"
data = "data"
resources = "resources"
addons = "addons"
```

#### JSON Format (`project.json`)

```json
{
  "project": {
    "name": "myProject",
    "version": "1.0.0",
    "author": "John Doe",
    "bundle_id": "com.example.myproject"
  },
  "entry": {
    "mode": "swiftui"
  },
  "addons": {
    "core": ["ofxOsc", "ofxGui"],
    "custom": [
      {
        "name": "ofxMyAddon",
        "mode": "copy",
        "source": "~/addons/ofxMyAddon"
      }
    ]
  },
  "build": {
    "cmake": true,
    "xcodegen": true,
    "min_macos": "13.0",
    "swift_version": "5.9",
    "cpp_standard": "c++20"
  }
}
```

#### Schema Validation

| Field | Type | Required | Default | Constraints |
|-------|------|----------|---------|-------------|
| `project.name` | string | ✅ | - | PascalCase or kebab-case |
| `project.version` | string | ❌ | `1.0.0` | Semantic versioning |
| `project.bundle_id` | string | ❌ | `com.example.<name>` | Reverse DNS format |
| `addons.core` | array | ❌ | `[]` | Valid Core/Native addon names |
| `addons.custom` | array | ❌ | `[]` | Objects with `name`, `mode`, `source` |
| `build.min_macos` | string | ❌ | `13.0` | >= 13.0 |
| `build.cpp_standard` | string | ❌ | `c++20` | `c++17` \| `c++20` |

---

### Templates

| Template | Description | Use Case |
|----------|-------------|----------|
| `basic` | 最小構成プロジェクト | 汎用的な開始点 |
| `swiftui` | SwiftUI 統合プロジェクト | macOS アプリ開発 |
| `metal` | Metal compute/shader プロジェクト | GPU 計算、シェーダー開発 |
| `3d` | 3D グラフィックスプロジェクト | 3D 可視化、モデルビューア |

---

## Build System Integration

### CMake Integration

#### Core/Native Addons (Reference)

```cmake
# oflike-metal をリンクすれば Core/Native Addons は利用可能
find_package(oflike-metal REQUIRED)
target_link_libraries(myProject oflike-metal::oflike-metal)
```

#### Custom Addons (Copy/Symlink)

```cmake
# カスタム addon をサブディレクトリとして追加
add_subdirectory(addons/ofxMyAddon)
target_link_libraries(myProject ofxMyAddon)

# ヘッダーパスを追加
target_include_directories(myProject PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/addons/ofxMyAddon/src
)
```

**Addon の CMakeLists.txt 例**:
```cmake
# addons/ofxMyAddon/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(ofxMyAddon)

# ソースファイル
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.mm")
file(GLOB_RECURSE HEADERS "src/*.h")

# スタティックライブラリとして構築
add_library(ofxMyAddon STATIC ${SOURCES} ${HEADERS})

# oflike-metal をリンク
target_link_libraries(ofxMyAddon oflike-metal::oflike-metal)

# ヘッダーパス
target_include_directories(ofxMyAddon PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
```

---

### XcodeGen Integration

#### Core/Native Addons (Reference)

```yaml
targets:
  myProject:
    dependencies:
      - framework: oflike-metal.framework
        embed: true
```

#### Custom Addons (Copy/Symlink)

```yaml
targets:
  myProject:
    sources:
      - src
      - addons/ofxMyAddon/src

    settings:
      HEADER_SEARCH_PATHS:
        - addons/ofxMyAddon/src

    # Addon が外部ライブラリを持つ場合
    dependencies:
      - target: ofxMyAddon
```

---

## Coding Standards

### Addon Structure

標準的な addon ディレクトリ構造:

```
ofxMyAddon/
├── src/                       # ソースコード
│   ├── ofxMyAddon.h          # メインヘッダー
│   ├── ofxMyAddon.cpp/.mm    # メイン実装
│   └── internal/             # 内部実装 (オプション)
│       ├── Helper.h
│       └── Helper.cpp
├── libs/                      # サードパーティライブラリ (オプション)
│   ├── mylib/
│   │   ├── include/
│   │   └── lib/
│   └── CMakeLists.txt
├── examples/                  # 使用例 (オプション)
│   └── example_basic/
├── docs/                      # ドキュメント (オプション)
│   └── README.md
├── CMakeLists.txt            # CMake ビルド設定
├── addon_config.yml          # Addon メタデータ
└── README.md                 # Addon 説明
```

### addon_config.yml

Addon のメタデータを定義:

```yaml
name: ofxMyAddon
version: 1.0.0
author: Your Name
description: My custom addon for oflike-metal
url: https://github.com/yourname/ofxMyAddon

dependencies:
  frameworks:
    - Metal
    - MetalKit
  addons:
    - ofxOsc  # 依存する他の addon

compatibility:
  minVersion: "1.0.0"  # 最小 oflike-metal バージョン
  platforms:
    - macOS

libs:
  - name: mylib
    path: libs/mylib
```

---

## Generator Constraints

### 1. Addon 命名規則

✅ **DO**:
- `ofx` で始まる (例: `ofxOsc`, `ofxMyAddon`)
- PascalCase を使用

❌ **DON'T**:
- Core/Native Addon と同じ名前を使用
- 特殊文字を含む名前

### 2. Core/Native Addons の扱い

✅ **DO**:
- Core/Native Addons は Reference モードのみ
- プロジェクトに Copy/Symlink しない

❌ **DON'T**:
- Core/Native Addons を `addons/` にコピー
- oflike-metal の addon を編集

### 3. プロジェクト構造

✅ **DO**:
- 標準的なフォルダ構造に従う (PROJECT_STRUCTURE.md 参照)
- CMake と XcodeGen の両方をサポート

❌ **DON'T**:
- Xcode プロジェクトファイル (.xcodeproj) を Git にコミット
- 非標準的なパス構造を使用

---

## Version History

| 日付 | バージョン | 変更内容 |
|------|-----------|---------|
| 2026-01-25 | 1.0.0 | 初版作成 (Phase 0.5) |
| 2026-01-25 | 1.0.1 | ofMain 非推奨と build flag を明記 |
| 2026-01-25 | 1.0.2 | ofMain 撤廃、SwiftUI 単一エントリへ統一 |

---

## See Also

- [ARCHITECTURE.md](ARCHITECTURE.md) - プロジェクトアーキテクチャ
- [PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md) - プロジェクト構造
- [MIGRATION.md](MIGRATION.md) - openFrameworks からの移行

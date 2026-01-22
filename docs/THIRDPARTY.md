# Third-Party Libraries

サードパーティライブラリの使用方針と管理方法です。

---

## 方針

### 基本原則

1. **Appleネイティブフレームワーク優先**: 可能な限りAppleのフレームワークを使用
2. **ヘッダーオンリー優先**: ビルドの複雑さを避けるため
3. **最小依存**: 必要最小限のライブラリのみ使用
4. **ライセンス確認**: 商用利用可能なライセンスのみ

---

## 使用するライブラリ

### 必須

| ライブラリ | バージョン | 用途 | ライセンス |
|-----------|-----------|------|-----------|
| **GLM** | 0.9.9+ | 数学（ベクトル、行列、クォータニオン） | MIT |
| **tess2** | 1.0 | ポリゴンテセレーション（ofPath用） | SGI Free Software License B |
| **utf8** | 2.3.4+ | UTF-8文字列処理 | Boost Software License |

### オプション（必要に応じて）

| ライブラリ | バージョン | 用途 | 代替 | ライセンス |
|-----------|-----------|------|------|-----------|
| nlohmann/json | 3.x | JSON解析 | NSJSONSerialization | MIT |
| pugixml | 1.x | XML解析 | NSXMLParser | MIT |
| fmt | 10.x | 文字列フォーマット | std::format (C++20) | MIT |

---

## 使用しないライブラリ（Appleフレームワークで置換）

| oFで使用 | 代替フレームワーク | 理由 |
|----------|-------------------|------|
| FreeImage | ImageIO / MTKTextureLoader | 画像読み込みはApple APIで十分 |
| FreeType | Core Text | macOSネイティブフォント |
| Cairo | Core Graphics / Metal | 2D描画 |
| GLEW | 不要 | Metal使用、OpenGL不要 |
| GLFW | AppKit / MetalKit | ウィンドウ管理 |
| curl | NSURLSession | ネットワーク |
| OpenSSL | Security.framework | 暗号化 |
| RtAudio | Core Audio / AVAudioEngine | オーディオ |
| libpng | ImageIO | PNG読み込み |
| zlib | Compression.framework | 圧縮 |
| Poco | Foundation | ファイル、ネットワーク等 |

---

## ディレクトリ構造

```
third_party/
├── glm/                    # 数学ライブラリ
│   └── glm/
│       ├── glm.hpp
│       ├── vec2.hpp
│       ├── vec3.hpp
│       ├── mat4x4.hpp
│       └── ...
├── tess2/                  # テセレーション
│   ├── tesselator.h
│   └── tesselator.c
├── utf8/                   # UTF-8処理
│   └── utf8.h
│   └── utf8/
│       └── ...
└── README.md               # このファイルへのリンク
```

---

## セットアップ手順

### 方法1: oFからコピー（推奨）

```bash
# GLM
cp -r references/of_v0.12.1_osx_release/libs/glm/include/glm third_party/glm/

# tess2
cp -r references/of_v0.12.1_osx_release/libs/tess2/include/* third_party/tess2/

# utf8
cp -r references/of_v0.12.1_osx_release/libs/utf8/include/* third_party/utf8/
```

### 方法2: 直接ダウンロード

```bash
# GLM (GitHub)
git clone --depth 1 https://github.com/g-truc/glm.git third_party/glm-repo
cp -r third_party/glm-repo/glm third_party/glm/
rm -rf third_party/glm-repo

# tess2 (GitHub)
git clone --depth 1 https://github.com/memononen/libtess2.git third_party/tess2-repo
cp third_party/tess2-repo/Source/* third_party/tess2/
cp third_party/tess2-repo/Include/* third_party/tess2/
rm -rf third_party/tess2-repo

# utf8 (GitHub)
git clone --depth 1 https://github.com/nemtrif/utfcpp.git third_party/utf8-repo
cp -r third_party/utf8-repo/source/* third_party/utf8/
rm -rf third_party/utf8-repo
```

---

## CMake統合

```cmake
# third_party/CMakeLists.txt

# GLM (header-only)
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm)

# tess2
add_library(tess2 STATIC
    tess2/tesselator.c
    tess2/bucketalloc.c
    tess2/dict.c
    tess2/geom.c
    tess2/mesh.c
    tess2/priorityq.c
    tess2/sweep.c
)
target_include_directories(tess2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tess2)

# utf8 (header-only)
add_library(utf8 INTERFACE)
target_include_directories(utf8 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/utf8)
```

メインCMakeLists.txtに追加:

```cmake
add_subdirectory(third_party)
target_link_libraries(${PROJECT_NAME} PRIVATE glm tess2 utf8)
```

---

## ライセンス

### GLM - MIT License

```
Copyright (c) 2005 - G-Truc Creation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction...
```

### tess2 - SGI Free Software License B

```
Copyright (C) 2011 by Mikko Mononen
Based on GLU libtess by Eric Veach, July 1994

Permission to use, copy, modify, and distribute this software for any purpose
with or without fee is hereby granted, provided that the above copyright
notice and this permission notice appear in all copies.
```

### utf8 - Boost Software License 1.0

```
Copyright 2006 Nemanja Trifunovic

Distributed under the Boost Software License, Version 1.0.
```

---

## 注意事項

1. **stb_image は使用禁止**: ARCHITECTURE.md の方針に従い、ImageIO/MTKTextureLoader を使用
2. **OpenGL関連は不要**: Metal を使用するため、GLEW等は不要
3. **バージョン管理**: ライブラリ更新時はこのファイルも更新すること

```markdown
# oflike-metal

**macOS ネイティブ C++ クリエイティブコーディングフレームワーク**

SwiftUI + Metal ベースで、openFrameworks 互換 API を提供します。

---

## 特徴

- openFrameworks と同じ API で書ける
- Metal による高速レンダリング
- macOS ネイティブ（SwiftUI 統合）
- Apple Silicon 最適化

```cpp
class ofApp : public ofBaseApp {
    void setup() override {
        ofSetBackgroundColor(30);
    }
    
    void draw() override {
        ofSetColor(255, 100, 100);
        ofDrawCircle(ofGetWidth()/2, ofGetHeight()/2, 100);
    }
};
```

---

## 要件

- macOS 13.0+ (Ventura)
- Xcode 15+

---

## ビルド

```bash
open oflike_metal.xcodeproj
# Cmd + R で実行
```

## ビルド (CLI)

```bash
./scripts/build_app.sh <AppName>
./scripts/run_app.sh <AppName>
# Metal のデバッグログを有効化する場合
./scripts/run_app.sh <AppName> --metal-debug
```

## ワンコマンド生成 & 実行 (CLI)

```bash
./scripts/new_and_run.sh <AppName>
# Metal のデバッグログを有効化する場合
./scripts/new_and_run.sh <AppName> --metal-debug
```

## GUI (Project Wizard)

```bash
./scripts/run_project_wizard.sh
```

## VS Code Tasks

- `Tasks: Run Task` から `oflike: build app` / `oflike: run app` / `oflike: run app (metal debug)` / `oflike: build & run`
- 実行時に `AppName` と `Debug/Release` を指定

---

## 座標系

- 2D: 左上原点、+X 右、+Y 下
- 3D: 右手座標系

---

## ドキュメント

- [ARCHITECTURE.md](docs/ARCHITECTURE.md) - 設計方針
- [CHECKLIST.md](docs/CHECKLIST.md) - 開発進捗

---

## ライセンス

MIT License
```

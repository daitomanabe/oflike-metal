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



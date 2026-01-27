# FIXPLANS.md

SwiftUI 描画が正常に動いたので、次に手を入れると良さそうな修正候補を整理します。

## Rendering / Runtime
- Metal シェーダーを事前コンパイルして app に同梱する（現在は起動時にソースからコンパイルしているため初回が重い）。
- DrawList が空のフレームが続く場合の警告を追加して、描画が止まる系の不具合を早期検出できるようにする。

## Generator / Templates
- 既存アプリを最新テンプレートに合わせる `oflike-gen validate --fix` を実装（MetalView.swift / PerformanceMonitor.swift / Bridging Header などの更新）。
- `SWIFT_OBJC_BRIDGING_HEADER` のパスが `apps/` 配下前提なので、ブリッジヘッダをプロジェクト内にコピーして相対パス固定にする。
- SwiftUI 用の `ofCreateApp` ヘルパーヘッダ（またはマクロ）を用意して、ユーザーが手で `extern "C"` を書かなくても良いようにする。
- `tools/project_generator/templates/*` と実生成物の差異をなくす（例: `ofApp.cpp.template` の `ofClear(50)` など）。

## Build Consistency
- `build/liboflike-metal.a` のデプロイターゲットを生成アプリと揃える（CMake の `CMAKE_OSX_DEPLOYMENT_TARGET` を 13.0 に固定するなど）。
- 生成時に静的ライブラリが古い場合を検知して警告する（`OFLBridge::renderFrame` などのシンボルチェック）。
- Xcode 26 では Metal Toolchain が別コンポーネントなので、ビルド前に `xcodebuild -showComponent metalToolchain` の `Status: installed` を確認するチェックを追加する（exit code だけだと未導入でも 0 が返る）。
- `xcodebuild -downloadComponent MetalToolchain` が失敗する場合に備え、Xcode Settings > Components での手動導入手順を明記する。
- `xcodebuild -showComponent` が installed でも `xcrun metal -v` が失敗するケースがあるため、`xcrun metal -v` を実チェックに追加する。失敗時は Metal.xctoolchain の実パスから `metal` を直接実行できるか検証し、必要なら Xcode の Toolchains に Metal.xctoolchain を認識させる手順を追記する。
- `xcodebuild` が `~/.cache/clang/ModuleCache` への書き込み権限で失敗する場合に備え、`MTL_COMPILER_FLAGS=-fmodules-cache-path=/tmp/oflike_metal_module_cache` を設定する手順を追加する（スペースを含まないパスに固定）。

## Warnings / Cleanup
- `MetalRenderer::getDevice()` / `getLastGPUTime()` に `override` を付けて警告を消す。
- `ofImage` / `ofUtilsImage` の `kUTType*` を `UTType` 系へ移行して非推奨警告を解消。

## Docs / Validation
- SwiftUI 経路の検証手順に `ofCreateApp()` の前提を明記。
- SwiftUI 検証サンプル（validation_swiftui）を最新の MetalView/ブリッジ構成に更新。

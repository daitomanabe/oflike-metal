# APIFIXPLANS.md

Core API changes needed for src/core alignment and restrictions.

## ofBaseApp / EventDispatcher
- Change `dragEvent` to accept a drag info struct (files + position) instead of just `(x, y)` to match openFrameworks semantics and enable file drops.
- Change `mouseScrolled` to use float `x`/`y` (currently int) to preserve precision from SwiftUI and align with openFrameworks.
- Add `windowMoved` (and optionally focus/lost-focus) events if openFrameworks compatibility is required by addons.

## Context
- Remove direct AppKit usage from `Context` (`src/core/Context.mm` uses `NSScreen` + `NSCursor`); replace with CoreGraphics (`CGDisplayBounds`, `CGDisplayHideCursor/ShowCursor`) or SwiftUI-provided setters/callbacks for screen metrics and cursor visibility.
- `getCurrentMatrix` currently returns identity; either wire it to the graphics matrix stack or remove/replace the API to avoid misleading state.

## Optional compatibility additions
- Add `gotMessage`/`messageReceived` and touch events (`touchDown`, `touchMoved`, `touchUp`, etc.) to match ofBaseApp if addon compatibility is a goal.

## oflike (legacy entry + system utils)
- Decision: remove the legacy `src/oflike/app/ofMain.h` entry path (AppKit-heavy: `NSApplication`, `NSWindow`, `NSMenu`, `MTKView` delegates) and unify on SwiftUI entry.
- `src/oflike/utils/ofUtilsSystem.mm` uses `NSWorkspace` (AppKit) for `ofLaunchBrowser`; replace with LaunchServices (`LSOpenCFURLRef`) or route through SwiftUI `openURL` via a platform callback.

## platform (SwiftUI)
- Decision: allow AppKit usage inside SwiftUI layer. Keep `NSApp.mainWindow`/`NSWindow` fullscreen toggling as-is.

## render
- No API-level changes found needed in `src/render` based on current constraints.

## Docs
- Document ofMain removal and SwiftUI-only entry in `docs/ARCHITECTURE.md` and `docs/IMPLEMENTATION.md`.
- If `ofLaunchBrowser` moves to LaunchServices or `Context` moves to CoreGraphics, add the required framework linkage (`CoreServices`/`ApplicationServices`) in CMake/XcodeGen.

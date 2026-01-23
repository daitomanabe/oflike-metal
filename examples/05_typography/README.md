# Example 05: Typography and Font Rendering

This example demonstrates comprehensive font rendering capabilities using `ofTrueTypeFont` with Core Text backend.

## Features Demonstrated

### 1. Font Loading
- **System Fonts**: Load macOS system fonts by name (e.g., "Helvetica", "Menlo")
- **Multiple Sizes**: Load same font at different point sizes
- **Font Types**:
  - Title font (Helvetica-Bold, 48pt)
  - Body font (Helvetica, 24pt)
  - Monospace font (Menlo, 16pt)
  - Japanese font (HiraginoSans-W3, 20pt)

### 2. Text Rendering
- **Basic Drawing**: `drawString(text, x, y)` - renders text at position
- **Baseline Positioning**: Y coordinate is the text baseline
- **Color Control**: Use `ofSetColor()` before drawing

### 3. Text Measurement
- **Width**: `stringWidth(text)` - get text width in pixels
- **Height**: `stringHeight(text)` - get text height (ascent + descent)
- **Bounding Box**: `getStringBoundingBox(text, x, y)` - get rectangle containing text
- Visual demonstration with red outline box

### 4. Letter Spacing
- **Get Spacing**: `getLetterSpacing()` - current spacing in pixels
- **Set Spacing**: `setLetterSpacing(pixels)` - adjust character spacing
- Examples:
  - Tight spacing: -2px
  - Normal spacing: 0px
  - Wide spacing: 8px

### 5. Line Height
- **Get Line Height**: `getLineHeight()` - baseline to baseline spacing
- **Set Line Height**: `setLineHeight(multiplier)` - adjust vertical spacing
- Multi-line text rendering with custom line spacing

### 6. Animated Effects
- **Wave Animation**: Characters move vertically in sine wave pattern
- **Rainbow Colors**: Each character cycles through HSB color space
- Demonstrates per-character rendering and measurement

### 7. Multi-line Text
- Rendering multiple lines with custom line spacing
- Paragraph-style text layout
- Line height multiplier demonstration

### 8. Japanese/Unicode Support
- UTF-8 string support
- Japanese Hiragana rendering: "日本語テキスト表示"
- Japanese greeting: "こんにちは、世界！"
- Uses HiraginoSans for proper glyph rendering

### 9. Monospace Code Display
- Code syntax examples with monospace font
- Fixed-width character alignment
- FPS and info display

## Visual Layout

```
┌─────────────────────────────────────────┐
│ Typography                    [Title]   │
│ Basic text rendering...       [Body]    │
│                                          │
│ Measured Text [with bbox]    [Measure]  │
│                                          │
│ Tight spacing                [Spacing]  │
│ Normal spacing                           │
│ Wide spacing                             │
│                                          │
│ WAVE ANIMATION                [Animated]│
│                                          │
│ Multi-line text rendering    [Multi]    │
│ Line height control                      │
│ Paragraph formatting                     │
│                                          │
│ 日本語テキスト表示              [Japanese]│
│ こんにちは、世界！                        │
│                                          │
│ // Code example...           [Mono]     │
│ ofTrueTypeFont font;                     │
│ font.load("Helvetica", 24);              │
│                                          │
│                         FPS: 60.0        │
│                         Fonts loaded: 4  │
│                         Core Text        │
└─────────────────────────────────────────┘
```

## API Reference

### ofTrueTypeFont Class

#### Loading
```cpp
bool load(const string& fontPath, int fontSize,
          bool antialiased = true,
          bool fullCharacterSet = false,
          bool makeContours = false,
          float dpi = 72.0f);
bool isLoaded() const;
```

#### Drawing
```cpp
void drawString(const string& text, float x, float y) const;
void drawStringAsShapes(const string& text, float x, float y) const;
```

#### Measurement
```cpp
float stringWidth(const string& text) const;
float stringHeight(const string& text) const;
ofRectangle getStringBoundingBox(const string& text, float x, float y) const;
```

#### Spacing Control
```cpp
float getLineHeight() const;
void setLineHeight(float lineHeight);
float getLetterSpacing() const;
void setLetterSpacing(float spacing);
float getFontSize() const;
```

#### Advanced
```cpp
vector<ofPath> getCharacterAsPoints(char32_t character, bool vflip = false) const;
vector<ofPath> getCharacterAsPoints(const string& character, bool vflip = false) const;
```

## Controls

| Key | Action |
|-----|--------|
| SPACE | Reset animation |

## Implementation Notes

### Core Text Backend
- **No FreeType**: Uses Apple's Core Text framework exclusively
- **System Fonts**: Access all macOS system fonts by name
- **Unicode Support**: Full UTF-8/UTF-32 support via Core Text
- **Glyph Atlas**: Efficient texture atlas for bitmap rendering
- **Antialiasing**: Core Graphics antialiasing for smooth text

### Font Names
Common macOS system fonts:
- `"Helvetica"` / `"Helvetica-Bold"`
- `"Menlo"` / `"Menlo-Bold"` (monospace)
- `"HiraginoSans-W3"` / `"HiraginoSans-W6"` (Japanese)
- `"SFPro"` / `"SFMono"` (San Francisco)
- `"TimesNewRomanPSMT"` (Times)

### Coordinate System
- **Origin**: Y coordinate is the text baseline
- **Direction**: Y increases downward (screen coordinates)
- **Positioning**: Use `getStringBoundingBox()` for precise layout

### Performance
- **Glyph Caching**: First render creates atlas, subsequent renders are fast
- **Batch Rendering**: DrawList batches text quads for GPU efficiency
- **Atlas Updates**: New characters dynamically added to atlas

## Usage

### Integration Steps
1. Copy `examples/05_typography/main.cpp` to your project
2. Link against oflike-metal library
3. Ensure Core Text framework is available (macOS 13.0+)
4. Run the example:
   ```bash
   cd build
   cmake ..
   make
   ./05_typography
   ```

### Custom Fonts
To use custom .ttf/.otf fonts:
```cpp
// Load from file path
font.load("/path/to/font.ttf", 24);

// Or load from Resources folder
font.load("Resources/CustomFont.ttf", 24);
```

## Learning Points

1. **Font Loading**: Use system font names or file paths
2. **Text Measurement**: Get accurate dimensions before rendering
3. **Letter Spacing**: Control character spacing for design effects
4. **Line Height**: Adjust vertical spacing for multi-line text
5. **Unicode Support**: UTF-8 strings work seamlessly
6. **Color Control**: Use `ofSetColor()` before drawing text
7. **Animation**: Render characters individually for per-char effects
8. **Performance**: First render caches glyphs, subsequent renders are fast

## Architecture Compliance

✅ **Core Text**: Uses Apple's Core Text framework exclusively
✅ **No FreeType**: No FreeType dependency
✅ **Pure C++**: Example code is pure C++
✅ **oF API Compatible**: Matches openFrameworks API exactly
✅ **Metal Backend**: Text rendered as textured quads via Metal
✅ **Unicode Support**: Full UTF-8/UTF-32 via Core Text
✅ **System Integration**: Native macOS font rendering

## Expected Output

The example displays:
- Large title in Helvetica Bold
- Body text with measurement visualization
- Letter spacing variations (tight/normal/wide)
- Animated wave text with rainbow colors
- Multi-line paragraph text
- Japanese Hiragana text
- Monospace code snippet
- FPS counter and info display

Console output:
```
05_typography example started
All fonts loaded successfully
Title font size: 48
Body font line height: 28.8
```

## Next Steps

After mastering this example:
- **06_3d_primitives**: 3D primitive rendering
- **07_camera**: Camera control and navigation
- **08_lighting**: Light sources and materials
- **Experiment**: Try different fonts, sizes, and effects
- **Custom Layout**: Build your own text layout engine
- **Rich Text**: Implement multi-font, multi-color text

# Example 03: Color Operations

Demonstrates comprehensive color manipulation capabilities in oflike-metal.

## Visual Layout

The example displays multiple rows of color demonstrations:

### Row 1: Predefined Colors
Shows all built-in color constants:
- `ofColor::white` - White (255, 255, 255)
- `ofColor::gray` - Gray (128, 128, 128)
- `ofColor::black` - Black (0, 0, 0)
- `ofColor::red` - Red (255, 0, 0)
- `ofColor::green` - Green (0, 255, 0)
- `ofColor::blue` - Blue (0, 0, 255)
- `ofColor::cyan` - Cyan (0, 255, 255)
- `ofColor::magenta` - Magenta (255, 0, 255)
- `ofColor::yellow` - Yellow (255, 255, 0)

### Row 2: HSB Hue Cycling
Demonstrates hue variation in HSB color space:
- 9 colors with different hues (0-255)
- Full saturation and brightness
- Animated cycling through the color spectrum
- Shows rainbow progression

### Row 3: Saturation Variation
Shows the effect of changing saturation:
- Same hue, full brightness
- Saturation from 0 (gray) to 255 (full color)
- Demonstrates desaturation effect

### Row 4: Brightness Variation
Shows the effect of changing brightness:
- Same hue, full saturation
- Brightness from dark to bright
- Demonstrates value changes

### Row 5: Hex Colors
Creates colors using hexadecimal color codes:
- `ofColor::fromHex(0xFF6B35)` - Orange
- `ofColor::fromHex(0xF7931E)` - Amber
- `ofColor::fromHex(0xFDC830)` - Gold
- `ofColor::fromHex(0xC5D86D)` - Yellow-green
- `ofColor::fromHex(0x83D475)` - Green
- `ofColor::fromHex(0x00B4D8)` - Light blue
- `ofColor::fromHex(0x0077B6)` - Medium blue
- `ofColor::fromHex(0x004E89)` - Dark blue
- `ofColor::fromHex(0x6A4C93)` - Purple

### Row 6: Color Interpolation (Lerp)
Demonstrates linear interpolation between colors:
- Source color 1 (orange)
- Source color 2 (blue)
- Animated lerp result (oscillates between sources)
- 6-step gradient showing lerp progression
- Uses `color1.lerp(color2, amount)`

### Row 7: Color Operations
Shows various color manipulation operations:
- **Base** - Original color (HSB animated)
- **Inv** - Inverted color using `getInverted()`
- ***0.5** - Darkened (multiply by 0.5)
- ***1.5** - Brightened and clamped (multiply by 1.5)
- **Add1** - Second color for addition
- **Sum** - Result of color addition (clamped)
- **Mul1** - First color for multiplication
- **Mul2** - Second color for multiplication
- **Prod** - Result of color multiplication

### Bottom: Animated Color Wave
Vertical bars with animated colors:
- Height varies with sine wave
- Color cycles through hue spectrum
- Brightness oscillates
- Creates flowing rainbow effect

## Interactive Controls

- **SPACE** - Reset animation to beginning

## API Functions Used

### Color Construction
- `ofColor(r, g, b)` - Create color from RGB values (0-255)
- `ofColor::fromHex(hexValue)` - Create from hex (e.g., 0xFF6B35)
- `ofColor::fromHsb(h, s, b)` - Create from HSB values (all 0-255)

### Color Constants
- `ofColor::white`, `gray`, `black`
- `ofColor::red`, `green`, `blue`
- `ofColor::cyan`, `magenta`, `yellow`

### Color Operations
- `color.lerp(target, amount)` - Linear interpolation (0.0 - 1.0)
- `color.getInverted()` - Get complementary color
- `color.getClamped()` - Clamp all channels to valid range
- `color * scalar` - Multiply all channels by scalar
- `color1 + color2` - Add colors component-wise
- `color1 * color2` - Multiply colors component-wise

### HSB Getters
- `color.getHue()` - Get hue value (0-255)
- `color.getSaturation()` - Get saturation (0-255)
- `color.getBrightness()` - Get brightness (0-255)

### HSB Setters
- `color.setHsb(h, s, b)` - Set from HSB values

### Drawing with Colors
- `ofSetColor(r, g, b)` - Set drawing color from RGB
- `ofSetColor(color.r, color.g, color.b)` - Set from ofColor
- `ofBackground(gray)` - Set background to gray value

## Key Concepts

### RGB vs HSB Color Spaces

**RGB (Red, Green, Blue)**:
- Additive color model (light-based)
- Each channel: 0-255
- Direct hardware representation
- Good for: exact color specification, image data

**HSB (Hue, Saturation, Brightness)**:
- Perceptual color model (human-friendly)
- Hue: 0-255 (color wheel position)
- Saturation: 0-255 (color intensity, 0=gray, 255=pure)
- Brightness: 0-255 (lightness, 0=black, 255=bright)
- Good for: color picking, animation, palettes

### Color Interpolation (Lerp)

Linear interpolation blends two colors:
```cpp
ofColor start = ofColor::fromHex(0xFF6B35);  // Orange
ofColor end = ofColor::fromHex(0x004E89);    // Blue
ofColor middle = start.lerp(end, 0.5f);      // 50% blend
```

- `amount = 0.0` returns start color
- `amount = 1.0` returns end color
- `amount = 0.5` returns middle blend
- Interpolates R, G, B, A channels separately

### Color Arithmetic

Colors support mathematical operations:

**Multiplication (scaling)**:
```cpp
ofColor color(200, 100, 50);
ofColor darker = color * 0.5f;    // (100, 50, 25)
ofColor brighter = color * 1.5f;  // (300, 150, 75) - needs clamping!
```

**Addition (brightening)**:
```cpp
ofColor color1(100, 0, 0);    // Red
ofColor color2(0, 100, 0);    // Green
ofColor result = color1 + color2;  // (100, 100, 0) - Yellow
```

**Clamping**:
```cpp
ofColor overflow(300, -50, 400);  // Invalid values
ofColor valid = overflow.getClamped();  // (255, 0, 255)
```

### Color Inversion

Creates complementary colors:
```cpp
ofColor color(200, 100, 50);
ofColor inverted = color.getInverted();  // (55, 155, 205)
```

Formula: `inverted = 255 - original` (for each RGB channel)
Alpha is not inverted.

## Integration Instructions

To run this example:

1. Build the oflike-metal library
2. In `src/platform/bridge/SwiftBridge.mm`, replace TestApp with:
   ```cpp
   #include "../../../examples/03_color/main.cpp"
   return new ColorOperationsApp();
   ```
3. Rebuild and run the application

## Expected Output

You should see:
- 9 rows of color boxes demonstrating different color techniques
- Row 1: Static predefined colors
- Rows 2-4: Animated HSB variations (hue cycling)
- Row 5: Static hex colors (design palette)
- Row 6: Animated lerp between orange and blue
- Row 7: Various color operations on animated base color
- Bottom: Flowing rainbow wave animation
- FPS indicator (green bar at bottom-right, 60fps target)

## Learning Points

1. **Multiple Color Creation Methods**: RGB, Hex, HSB - choose based on use case
2. **HSB for Animation**: Easier to create smooth color transitions
3. **Color Interpolation**: Create gradients and smooth transitions
4. **Color Operations**: Mathematical color manipulation for effects
5. **Clamping**: Always clamp after arithmetic to avoid invalid values
6. **Predefined Colors**: Convenient constants for common colors
7. **Animation Techniques**: Time-based hue cycling, sine waves

## Architecture Notes

This example is built on:
- **ofColor** - Template color class with uint8_t (0-255)
- **ofGraphics** - Drawing functions (ofDrawRectangle, ofSetColor)
- **ofBaseApp** - Application lifecycle (setup, update, draw)
- Pure C++ - No direct Metal/SwiftUI usage in example code

## openFrameworks API Compatibility

All functions used are **Level A** compatible with openFrameworks:
- ✅ `ofColor` constructor and member access (r, g, b, a)
- ✅ `ofColor::fromHex()` - Static factory method
- ✅ `ofColor::fromHsb()` - Static factory method
- ✅ `ofColor::red`, `green`, `blue`, etc. - Predefined constants
- ✅ `color.lerp()` - Linear interpolation
- ✅ `color.getInverted()` - Color inversion
- ✅ `color.getClamped()` - Clamp to valid range
- ✅ `color.getHue()`, `getSaturation()`, `getBrightness()` - HSB getters
- ✅ `color.setHsb()` - HSB setter
- ✅ Color operators: `+`, `-`, `*`, `/`
- ✅ `ofSetColor(r, g, b)` - Set drawing color
- ✅ `ofBackground()` - Clear screen
- ✅ `ofDrawRectangle()` - Draw shapes
- ✅ `ofFill()`, `ofNoFill()` - Fill mode
- ✅ `ofGetFrameRate()` - Get current FPS

This example exactly matches openFrameworks color API behavior.

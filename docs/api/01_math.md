# Math API - Vector and Matrix Types

## Overview

oflike-metal provides a comprehensive math library fully compatible with openFrameworks, backed by Apple's high-performance SIMD library. All math types use `simd` internally for optimal performance on Apple Silicon and Intel Macs.

## Architecture

- **simd Backend**: All types use Apple's simd library (not GLM)
- **API Compatibility**: 100% compatible with openFrameworks math API
- **Performance**: Leverages hardware SIMD instructions
- **Coordinate System**: Right-handed (3D), top-left origin (2D)

---

## ofVec2f - 2D Vector

### Header

```cpp
#include <oflike/math/ofVec2f.h>
```

### Construction

```cpp
ofVec2f()                    // (0, 0)
ofVec2f(float x, float y)    // Specific x, y
ofVec2f(float scalar)        // (scalar, scalar)
ofVec2f(simd_float2)         // From simd type
```

### Member Variables

```cpp
float x, y;
```

### Arithmetic Operations

```cpp
ofVec2f a(10, 20);
ofVec2f b(5, 10);

ofVec2f c = a + b;       // Vector addition
ofVec2f d = a - b;       // Vector subtraction
ofVec2f e = a * 2.0f;    // Scalar multiplication
ofVec2f f = a / 2.0f;    // Scalar division
ofVec2f g = -a;          // Negation

a += b;  // In-place operations
a *= 2.0f;
```

### Vector Math

```cpp
float len = v.length();              // Magnitude
float lenSq = v.lengthSquared();     // Squared magnitude (faster)

v.normalize();                       // Normalize to unit length
ofVec2f unit = v.getNormalized();    // Get normalized copy

v.scale(100);                        // Scale to specific length
ofVec2f scaled = v.getScaled(100);   // Get scaled copy

v.limit(50);                         // Limit maximum length
ofVec2f limited = v.getLimited(50);  // Get limited copy

float dot = a.dot(b);                // Dot product
```

### Distance

```cpp
float dist = a.distance(b);          // Euclidean distance
float distSq = a.squareDistance(b);  // Squared distance (faster)
```

### Setters

```cpp
v.set(10, 20);          // Set x, y
v.set(5);               // Set both to 5
v.set(other);           // Copy from other
```

### Array Access

```cpp
v[0] = 10;              // v.x
v[1] = 20;              // v.y
float* ptr = v.getPtr();
```

### Static Helpers

```cpp
ofVec2f zero = ofVec2f::zero();      // (0, 0)
ofVec2f one = ofVec2f::one();        // (1, 1)
```

### Example

```cpp
ofVec2f pos(100, 100);
ofVec2f velocity(2, 1);

pos += velocity;                     // Move
float speed = velocity.length();     // Get speed

ofVec2f direction = velocity.getNormalized();  // Unit direction
```

---

## ofVec3f - 3D Vector

### Header

```cpp
#include <oflike/math/ofVec3f.h>
```

### Construction

```cpp
ofVec3f()                           // (0, 0, 0)
ofVec3f(float x, float y, float z)  // Specific x, y, z
ofVec3f(float scalar)               // (scalar, scalar, scalar)
ofVec3f(simd_float3)                // From simd type
```

### Member Variables

```cpp
float x, y, z;
```

### Arithmetic Operations

Same as ofVec2f, with 3 components.

### Vector Math

```cpp
// All ofVec2f operations plus:

float dot = a.dot(b);                   // Dot product
ofVec3f cross = a.cross(b);             // Cross product (3D specific)
ofVec3f crossed = a.getCrossed(b);      // Get cross product

ofVec3f perp = v.getPerpendicular();    // Get perpendicular vector
```

### Interpolation

```cpp
a.interpolate(b, 0.5f);                 // Lerp 50% toward b
ofVec3f mid = a.getInterpolated(b, 0.5f);

ofVec3f midpoint = a.getMiddle(b);      // Midpoint between a and b
a.middle(b);                            // Set to midpoint
```

### Rotation

```cpp
v.rotate(45, ofVec3f(0, 1, 0));         // Rotate 45° around Y axis
v.rotateRad(PI/4, axis);                // Rotate in radians

ofVec3f rotated = v.getRotated(45, axis);
ofVec3f rotatedRad = v.getRotatedRad(PI/4, axis);
```

### Angles

```cpp
float angleDeg = a.angle(b);            // Angle in degrees
float angleRad = a.angleRad(b);         // Angle in radians
```

### Comparison

```cpp
bool equal = a == b;
bool similar = a.match(b, 0.001f);      // Within tolerance
```

### Example

```cpp
ofVec3f position(0, 0, 0);
ofVec3f forward(0, 0, -1);
ofVec3f up(0, 1, 0);

ofVec3f right = forward.cross(up);      // Right vector
right.normalize();

position += forward * speed;            // Move forward

float angle = forward.angle(target);     // Angle to target
```

---

## ofVec4f - 4D Vector

### Header

```cpp
#include <oflike/math/ofVec4f.h>
```

### Construction

```cpp
ofVec4f()                               // (0, 0, 0, 0)
ofVec4f(float x, float y, float z, float w)
ofVec4f(float scalar)                   // All components = scalar
ofVec4f(simd_float4)
```

### Member Variables

```cpp
float x, y, z, w;
```

Similar operations to ofVec3f but with 4 components. Commonly used for homogeneous coordinates and RGBA colors.

---

## ofMatrix4x4 - 4x4 Transformation Matrix

### Header

```cpp
#include <oflike/math/ofMatrix4x4.h>
```

### Construction

```cpp
ofMatrix4x4()                           // Identity matrix
ofMatrix4x4(simd_float4x4)              // From simd type
ofMatrix4x4(16 floats...)               // Row-major order
```

### Static Factories

```cpp
// Basic matrices
ofMatrix4x4 identity = ofMatrix4x4::identity();
ofMatrix4x4 zero = ofMatrix4x4::zero();

// Transformation matrices
ofMatrix4x4 trans = ofMatrix4x4::newTranslationMatrix(10, 20, 30);
ofMatrix4x4 rot = ofMatrix4x4::newRotationMatrix(45, 0, 1, 0);  // 45° around Y
ofMatrix4x4 scale = ofMatrix4x4::newScaleMatrix(2, 2, 2);

// From vectors
ofMatrix4x4 trans = ofMatrix4x4::newTranslationMatrix(ofVec3f(10, 20, 30));
ofMatrix4x4 rot = ofMatrix4x4::newRotationMatrix(45, ofVec3f(0, 1, 0));
ofMatrix4x4 scale = ofMatrix4x4::newScaleMatrix(ofVec3f(2, 2, 2));

// Projection matrices
ofMatrix4x4 persp = ofMatrix4x4::newPerspectiveMatrix(
    60.0f,      // FOV in degrees
    16.0f/9.0f, // Aspect ratio
    0.1f,       // Near plane
    1000.0f     // Far plane
);

ofMatrix4x4 ortho = ofMatrix4x4::newOrthoMatrix(
    0, 800,     // left, right
    0, 600,     // bottom, top
    -1, 1       // near, far
);

ofMatrix4x4 ortho2d = ofMatrix4x4::newOrtho2DMatrix(0, 800, 0, 600);

// View matrix
ofMatrix4x4 view = ofMatrix4x4::newLookAtMatrix(
    ofVec3f(0, 0, 10),  // eye position
    ofVec3f(0, 0, 0),   // look at point
    ofVec3f(0, 1, 0)    // up vector
);
```

### Matrix Operations

```cpp
ofMatrix4x4 result = a * b;             // Matrix multiplication
result *= b;                            // In-place multiplication

ofVec3f transformed = mat * point;      // Transform point
ofVec4f transformed = mat * vec4;       // Transform 4D vector
```

### Transformations

```cpp
mat.translate(10, 20, 30);              // Apply translation
mat.translate(ofVec3f(10, 20, 30));

mat.rotate(45, 0, 1, 0);                // Rotate 45° around Y
mat.rotateX(45);                        // Rotate around X
mat.rotateY(45);                        // Rotate around Y
mat.rotateZ(45);                        // Rotate around Z

mat.scale(2, 2, 2);                     // Apply scale
mat.scale(2);                           // Uniform scale
```

### Matrix Math

```cpp
mat.transpose();                        // Transpose in-place
ofMatrix4x4 t = mat.getTransposed();    // Get transposed copy

mat.invert();                           // Invert in-place
ofMatrix4x4 inv = mat.getInverse();     // Get inverted copy

float det = mat.determinant();          // Get determinant
```

### Decomposition

```cpp
ofVec3f translation = mat.getTranslation();
ofVec3f scale = mat.getScale();

mat.setTranslation(10, 20, 30);         // Set translation component
```

### Pre/Post Multiplication

```cpp
mat.preMult(other);                     // other * mat (left multiply)
mat.postMult(other);                    // mat * other (right multiply)
```

### Element Access

```cpp
float value = mat(row, col);            // Read element (row-major indexing)
mat.set(row, col, value);               // Write element

float* ptr = mat.getPtr();              // Pointer to data (column-major)
const float* ptr = mat.getPtr();
```

### Example

```cpp
// Model transformation
ofMatrix4x4 model;
model.translate(100, 50, 0);
model.rotateZ(45);
model.scale(2);

// View transformation
ofMatrix4x4 view = ofMatrix4x4::newLookAtMatrix(
    ofVec3f(0, 0, 10),  // Camera position
    ofVec3f(0, 0, 0),   // Look at origin
    ofVec3f(0, 1, 0)    // Up is Y
);

// Projection
ofMatrix4x4 proj = ofMatrix4x4::newPerspectiveMatrix(
    60, 16.0f/9.0f, 0.1f, 1000.0f
);

// Combined MVP
ofMatrix4x4 mvp = proj * view * model;

// Transform a point
ofVec3f worldPoint(10, 20, 30);
ofVec3f screenPoint = mvp * worldPoint;
```

---

## ofQuaternion - Rotation Quaternion

### Header

```cpp
#include <oflike/math/ofQuaternion.h>
```

Quaternions provide smooth rotation interpolation and avoid gimbal lock.

### Construction

```cpp
ofQuaternion()                          // Identity (no rotation)
ofQuaternion(float angle, ofVec3f axis) // From axis-angle (degrees)
ofQuaternion(simd_quatf)                // From simd quaternion
```

### Operations

```cpp
ofQuaternion result = q1 * q2;          // Combine rotations
ofVec3f rotated = q * vector;           // Rotate vector

q.makeRotate(angle, axis);              // Set rotation
ofMatrix4x4 mat = q.getMatrix();        // Convert to matrix

// Spherical interpolation
ofQuaternion slerp = q1.slerp(t, q2);   // Smooth interpolation
```

---

## ofMath - Math Utilities

### Header

```cpp
#include <oflike/math/ofMath.h>
```

### Constants

```cpp
float pi = PI;              // 3.14159265358979323846
float two_pi = TWO_PI;      // 6.28318530717958647693
float half_pi = HALF_PI;    // 1.57079632679489661923
float deg_to_rad = DEG_TO_RAD;  // 0.017453292519943295
float rad_to_deg = RAD_TO_DEG;  // 57.29577951308232
```

### Angle Conversion

```cpp
float rad = ofDegToRad(degrees);
float deg = ofRadToDeg(radians);
```

### Range Mapping

```cpp
float mapped = ofMap(value, inputMin, inputMax, outputMin, outputMax);
float clamped = ofClamp(value, min, max);
float lerped = ofLerp(start, stop, amt);
```

### Random

```cpp
float r = ofRandom(max);                // 0 to max
float r = ofRandom(min, max);           // min to max
int seed = ofRandomSeed();
ofRandomSeed(seed);
```

### Noise

```cpp
float n = ofNoise(x);                   // 1D Perlin noise
float n = ofNoise(x, y);                // 2D
float n = ofNoise(x, y, z);             // 3D
float n = ofSignedNoise(x);             // -1 to 1
```

---

## simd Interoperability

All types provide seamless conversion to/from Apple simd types:

```cpp
ofVec3f v(1, 2, 3);
simd_float3 s = v.toSimd();             // Explicit conversion
simd_float3 s2 = v;                     // Implicit conversion

ofVec3f v2 = ofVec3f::fromSimd(s);      // From simd
ofVec3f v3(s);                          // Direct construction

// Matrix
ofMatrix4x4 mat;
simd_float4x4 simdMat = mat.toSimd();
ofMatrix4x4 mat2(simdMat);
```

---

## Performance Notes

- **simd-backed**: All operations use hardware SIMD when possible
- **lengthSquared()**: Prefer over length() when comparing distances (avoids sqrt)
- **normalize()**: In-place is faster than getNormalized()
- **Column-major**: Matrices stored in OpenGL/Metal format

---

## See Also

- [00_foundation.md](00_foundation.md) - Core classes
- [02_graphics_2d.md](02_graphics_2d.md) - Using math in 2D graphics
- [03_graphics_3d.md](03_graphics_3d.md) - Using math in 3D graphics
- [06_camera.md](06_camera.md) - Camera and view matrices
- [ARCHITECTURE.md](../ARCHITECTURE.md) - simd usage guidelines

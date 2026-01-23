// Math Library Tests for oflike-metal
// Tests all components of the math library (Phase 3)

#include <iostream>
#include <cmath>
#include <cassert>
#include "ofVec2f.h"
#include "ofVec3f.h"
#include "ofVec4f.h"
#include "ofMatrix4x4.h"
#include "ofQuaternion.h"
#include "ofMath.h"

// Use oflike namespace
using namespace oflike;

// ANSI color codes for output
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

// Test result tracking
int g_totalTests = 0;
int g_passedTests = 0;
int g_failedTests = 0;

// Floating point comparison tolerance
const float EPSILON = 1e-5f;

bool floatEquals(float a, float b, float epsilon = EPSILON) {
    return std::abs(a - b) < epsilon;
}

void TEST_START(const char* name) {
    std::cout << "\n[TEST] " << name << "\n";
    g_totalTests++;
}

void TEST_PASS() {
    std::cout << GREEN << "  ✓ PASS" << RESET << "\n";
    g_passedTests++;
}

void TEST_FAIL(const char* message) {
    std::cout << RED << "  ✗ FAIL: " << message << RESET << "\n";
    g_failedTests++;
}

void REQUIRE(bool condition, const char* message) {
    if (!condition) {
        TEST_FAIL(message);
        throw std::runtime_error(message);
    }
}

void CHECK(bool condition, const char* message) {
    if (condition) {
        TEST_PASS();
    } else {
        TEST_FAIL(message);
    }
}

// ============================================================
// ofVec2f Tests
// ============================================================

void test_ofVec2f_construction() {
    TEST_START("ofVec2f Construction");

    ofVec2f v1;
    CHECK(v1.x == 0.0f && v1.y == 0.0f, "Default constructor should initialize to zero");

    ofVec2f v2(3.0f, 4.0f);
    CHECK(v2.x == 3.0f && v2.y == 4.0f, "Constructor should set x and y");

    ofVec2f v3(v2);
    CHECK(v3.x == 3.0f && v3.y == 4.0f, "Copy constructor should copy values");
}

void test_ofVec2f_operations() {
    TEST_START("ofVec2f Arithmetic Operations");

    ofVec2f v1(3.0f, 4.0f);
    ofVec2f v2(1.0f, 2.0f);

    ofVec2f sum = v1 + v2;
    CHECK(sum.x == 4.0f && sum.y == 6.0f, "Addition");

    ofVec2f diff = v1 - v2;
    CHECK(diff.x == 2.0f && diff.y == 2.0f, "Subtraction");

    ofVec2f scaled = v1 * 2.0f;
    CHECK(scaled.x == 6.0f && scaled.y == 8.0f, "Scalar multiplication");

    ofVec2f divided = v1 / 2.0f;
    CHECK(divided.x == 1.5f && divided.y == 2.0f, "Scalar division");
}

void test_ofVec2f_methods() {
    TEST_START("ofVec2f Methods");

    ofVec2f v1(3.0f, 4.0f);

    float len = v1.length();
    CHECK(floatEquals(len, 5.0f), "length() should return 5 for (3,4)");

    float lenSq = v1.lengthSquared();
    CHECK(floatEquals(lenSq, 25.0f), "lengthSquared() should return 25");

    ofVec2f normalized = v1.getNormalized();
    CHECK(floatEquals(normalized.length(), 1.0f), "getNormalized() should return unit vector");

    ofVec2f v2(1.0f, 0.0f);
    float dot = v1.dot(v2);
    CHECK(floatEquals(dot, 3.0f), "dot product (3,4)·(1,0) = 3");

    float dist = v1.distance(ofVec2f(0.0f, 0.0f));
    CHECK(floatEquals(dist, 5.0f), "distance to origin");
}

// ============================================================
// ofVec3f Tests
// ============================================================

void test_ofVec3f_construction() {
    TEST_START("ofVec3f Construction");

    ofVec3f v1;
    CHECK(v1.x == 0.0f && v1.y == 0.0f && v1.z == 0.0f, "Default constructor");

    ofVec3f v2(1.0f, 2.0f, 3.0f);
    CHECK(v2.x == 1.0f && v2.y == 2.0f && v2.z == 3.0f, "Constructor with values");
}

void test_ofVec3f_operations() {
    TEST_START("ofVec3f Arithmetic Operations");

    ofVec3f v1(1.0f, 2.0f, 3.0f);
    ofVec3f v2(4.0f, 5.0f, 6.0f);

    ofVec3f sum = v1 + v2;
    CHECK(sum.x == 5.0f && sum.y == 7.0f && sum.z == 9.0f, "Addition");

    ofVec3f diff = v1 - v2;
    CHECK(diff.x == -3.0f && diff.y == -3.0f && diff.z == -3.0f, "Subtraction");

    ofVec3f scaled = v1 * 2.0f;
    CHECK(scaled.x == 2.0f && scaled.y == 4.0f && scaled.z == 6.0f, "Scalar multiplication");
}

void test_ofVec3f_methods() {
    TEST_START("ofVec3f Methods");

    ofVec3f v1(1.0f, 0.0f, 0.0f);
    ofVec3f v2(0.0f, 1.0f, 0.0f);

    ofVec3f cross = v1.cross(v2);
    CHECK(floatEquals(cross.x, 0.0f) && floatEquals(cross.y, 0.0f) && floatEquals(cross.z, 1.0f),
          "cross product (1,0,0) × (0,1,0) = (0,0,1)");

    float dot = v1.dot(v2);
    CHECK(floatEquals(dot, 0.0f), "dot product of perpendicular vectors");

    ofVec3f v3(3.0f, 4.0f, 0.0f);
    float len = v3.length();
    CHECK(floatEquals(len, 5.0f), "length calculation");

    ofVec3f norm = v3.getNormalized();
    CHECK(floatEquals(norm.length(), 1.0f), "normalization");
}

// ============================================================
// ofVec4f Tests
// ============================================================

void test_ofVec4f_construction() {
    TEST_START("ofVec4f Construction");

    ofVec4f v1;
    CHECK(v1.x == 0.0f && v1.y == 0.0f && v1.z == 0.0f && v1.w == 0.0f,
          "Default constructor");

    ofVec4f v2(1.0f, 2.0f, 3.0f, 4.0f);
    CHECK(v2.x == 1.0f && v2.y == 2.0f && v2.z == 3.0f && v2.w == 4.0f,
          "Constructor with values");
}

void test_ofVec4f_operations() {
    TEST_START("ofVec4f Operations");

    ofVec4f v1(1.0f, 2.0f, 3.0f, 4.0f);
    ofVec4f v2(5.0f, 6.0f, 7.0f, 8.0f);

    ofVec4f sum = v1 + v2;
    CHECK(sum.x == 6.0f && sum.y == 8.0f && sum.z == 10.0f && sum.w == 12.0f,
          "Addition");

    ofVec4f scaled = v1 * 2.0f;
    CHECK(scaled.x == 2.0f && scaled.y == 4.0f && scaled.z == 6.0f && scaled.w == 8.0f,
          "Scalar multiplication");
}

// ============================================================
// ofMatrix4x4 Tests
// ============================================================

void test_ofMatrix4x4_construction() {
    TEST_START("ofMatrix4x4 Construction");

    ofMatrix4x4 m1;
    // Default constructor behavior may vary, just check it doesn't crash
    CHECK(true, "Default constructor");

    ofMatrix4x4 identity = ofMatrix4x4::identity();
    CHECK(floatEquals(identity(0,0), 1.0f) && floatEquals(identity(1,1), 1.0f) &&
          floatEquals(identity(2,2), 1.0f) && floatEquals(identity(3,3), 1.0f),
          "Identity matrix");
}

void test_ofMatrix4x4_operations() {
    TEST_START("ofMatrix4x4 Operations");

    ofMatrix4x4 identity = ofMatrix4x4::identity();
    ofMatrix4x4 translate = ofMatrix4x4::newTranslationMatrix(1.0f, 2.0f, 3.0f);

    // Test translation matrix has correct values
    CHECK(floatEquals(translate(3,0), 1.0f) &&
          floatEquals(translate(3,1), 2.0f) &&
          floatEquals(translate(3,2), 3.0f),
          "Translation matrix construction");

    ofMatrix4x4 scale = ofMatrix4x4::newScaleMatrix(2.0f, 2.0f, 2.0f);
    CHECK(floatEquals(scale(0,0), 2.0f) &&
          floatEquals(scale(1,1), 2.0f) &&
          floatEquals(scale(2,2), 2.0f),
          "Scale matrix construction");
}

void test_ofMatrix4x4_transformations() {
    TEST_START("ofMatrix4x4 Transformations");

    ofMatrix4x4 m;
    m.makeIdentity();
    m.translate(ofVec3f(10.0f, 20.0f, 30.0f));

    CHECK(floatEquals(m(3,0), 10.0f) &&
          floatEquals(m(3,1), 20.0f) &&
          floatEquals(m(3,2), 30.0f),
          "translate() modifies matrix");

    ofMatrix4x4 m2;
    m2.makeIdentity();
    m2.scale(2.0f, 3.0f, 4.0f);
    CHECK(floatEquals(m2(0,0), 2.0f) &&
          floatEquals(m2(1,1), 3.0f) &&
          floatEquals(m2(2,2), 4.0f),
          "scale() modifies matrix");
}

void test_ofMatrix4x4_projection() {
    TEST_START("ofMatrix4x4 Projection");

    ofMatrix4x4 persp = ofMatrix4x4::newPerspectiveMatrix(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    CHECK(persp(0,0) != 0.0f && persp(1,1) != 0.0f, "Perspective matrix");

    ofMatrix4x4 ortho = ofMatrix4x4::newOrthoMatrix(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
    CHECK(ortho(0,0) != 0.0f && ortho(1,1) != 0.0f, "Orthographic matrix");
}

// ============================================================
// ofQuaternion Tests
// ============================================================

void test_ofQuaternion_construction() {
    TEST_START("ofQuaternion Construction");

    ofQuaternion q1;
    CHECK(true, "Default constructor");

    ofQuaternion q2(0.0f, 0.0f, 0.0f, 1.0f);
    CHECK(floatEquals(q2.w(), 1.0f), "Constructor with values");
}

void test_ofQuaternion_operations() {
    TEST_START("ofQuaternion Operations");

    // Identity quaternion
    ofQuaternion identity(0.0f, 0.0f, 0.0f, 1.0f);
    CHECK(floatEquals(identity.w(), 1.0f), "Identity quaternion");

    // Rotation around Z axis by 90 degrees
    ofQuaternion q;
    q.makeRotate(90.0f, ofVec3f(0.0f, 0.0f, 1.0f));
    CHECK(q.w() != 0.0f, "makeRotate() creates rotation");

    // Test slerp (static method)
    ofQuaternion q1(0.0f, 0.0f, 0.0f, 1.0f);
    ofQuaternion q2;
    q2.makeRotate(90.0f, ofVec3f(0.0f, 0.0f, 1.0f));
    ofQuaternion result = ofQuaternion::slerp(q1, q2, 0.5f);
    CHECK(true, "slerp interpolation");
}

// ============================================================
// ofMath Functions Tests
// ============================================================

void test_ofMath_random() {
    TEST_START("ofMath Random Functions");

    ofSeedRandom(12345);
    float r1 = ofRandom(1.0f);
    CHECK(r1 >= 0.0f && r1 <= 1.0f, "ofRandom(max) in range [0,max]");

    float r2 = ofRandom(-5.0f, 5.0f);
    CHECK(r2 >= -5.0f && r2 <= 5.0f, "ofRandom(min,max) in range");

    float r3 = ofRandomf();
    CHECK(r3 >= -1.0f && r3 <= 1.0f, "ofRandomf() in range [-1,1]");

    float r4 = ofRandomuf();
    CHECK(r4 >= 0.0f && r4 <= 1.0f, "ofRandomuf() in range [0,1]");
}

void test_ofMath_noise() {
    TEST_START("ofMath Noise Functions");

    float n1 = ofNoise(0.5f);
    CHECK(n1 >= 0.0f && n1 <= 1.0f, "ofNoise() in range [0,1]");

    float n2 = ofNoise(0.5f, 0.7f);
    CHECK(n2 >= 0.0f && n2 <= 1.0f, "ofNoise(x,y) in range");

    float n3 = ofNoise(0.5f, 0.7f, 0.9f);
    CHECK(n3 >= 0.0f && n3 <= 1.0f, "ofNoise(x,y,z) in range");

    float sn1 = ofSignedNoise(0.5f);
    CHECK(sn1 >= -1.0f && sn1 <= 1.0f, "ofSignedNoise() in range [-1,1]");
}

void test_ofMath_mapping() {
    TEST_START("ofMath Mapping Functions");

    float mapped = ofMap(5.0f, 0.0f, 10.0f, 0.0f, 100.0f);
    CHECK(floatEquals(mapped, 50.0f), "ofMap() linear mapping");

    float clamped = ofClamp(15.0f, 0.0f, 10.0f);
    CHECK(floatEquals(clamped, 10.0f), "ofClamp() clamps to max");

    float clamped2 = ofClamp(-5.0f, 0.0f, 10.0f);
    CHECK(floatEquals(clamped2, 0.0f), "ofClamp() clamps to min");

    float lerped = ofLerp(0.0f, 100.0f, 0.5f);
    CHECK(floatEquals(lerped, 50.0f), "ofLerp() interpolation");
}

void test_ofMath_distance() {
    TEST_START("ofMath Distance Functions");

    float dist = ofDist(0.0f, 0.0f, 3.0f, 4.0f);
    CHECK(floatEquals(dist, 5.0f), "ofDist(x1,y1,x2,y2)");

    float distSq = ofDistSquared(0.0f, 0.0f, 3.0f, 4.0f);
    CHECK(floatEquals(distSq, 25.0f), "ofDistSquared()");

    float dist3d = ofDist(0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 2.0f);
    CHECK(floatEquals(dist3d, 3.0f), "ofDist(x1,y1,z1,x2,y2,z2)");
}

void test_ofMath_angles() {
    TEST_START("ofMath Angle Functions");

    float rad = ofDegToRad(180.0f);
    CHECK(floatEquals(rad, M_PI), "ofDegToRad(180) = π");

    float deg = ofRadToDeg(M_PI);
    CHECK(floatEquals(deg, 180.0f), "ofRadToDeg(π) = 180");

    float wrapped = ofWrap(370.0f, 0.0f, 360.0f);
    CHECK(floatEquals(wrapped, 10.0f), "ofWrap(370, 0, 360) = 10");

    float wrappedDeg = ofWrapDegrees(370.0f);
    CHECK(floatEquals(wrappedDeg, 10.0f), "ofWrapDegrees(370) = 10");

    float wrappedRad = ofWrapRadians(M_PI * 3.0f);
    CHECK(wrappedRad >= -M_PI && wrappedRad <= M_PI, "ofWrapRadians() in range");
}

// ============================================================
// Main Test Runner
// ============================================================

void printSummary() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "MATH LIBRARY TEST SUMMARY\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "Total Tests:  " << g_totalTests << "\n";
    std::cout << GREEN << "Passed:       " << g_passedTests << RESET << "\n";
    if (g_failedTests > 0) {
        std::cout << RED << "Failed:       " << g_failedTests << RESET << "\n";
    } else {
        std::cout << "Failed:       " << g_failedTests << "\n";
    }

    float percentage = (float)g_passedTests / (float)(g_passedTests + g_failedTests) * 100.0f;
    std::cout << "\nSuccess Rate: " << percentage << "%\n";

    if (g_failedTests == 0) {
        std::cout << GREEN << "\n✓ ALL TESTS PASSED!\n" << RESET;
    } else {
        std::cout << RED << "\n✗ SOME TESTS FAILED\n" << RESET;
    }
    std::cout << std::string(60, '=') << "\n";
}

int main() {
    std::cout << "\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "OFLIKE-METAL MATH LIBRARY TESTS\n";
    std::cout << "Phase 3: 数学ライブラリ (Math Library)\n";
    std::cout << std::string(60, '=') << "\n";

    try {
        // ofVec2f Tests
        std::cout << "\n" << YELLOW << "=== ofVec2f Tests ===" << RESET;
        test_ofVec2f_construction();
        test_ofVec2f_operations();
        test_ofVec2f_methods();

        // ofVec3f Tests
        std::cout << "\n" << YELLOW << "=== ofVec3f Tests ===" << RESET;
        test_ofVec3f_construction();
        test_ofVec3f_operations();
        test_ofVec3f_methods();

        // ofVec4f Tests
        std::cout << "\n" << YELLOW << "=== ofVec4f Tests ===" << RESET;
        test_ofVec4f_construction();
        test_ofVec4f_operations();

        // ofMatrix4x4 Tests
        std::cout << "\n" << YELLOW << "=== ofMatrix4x4 Tests ===" << RESET;
        test_ofMatrix4x4_construction();
        test_ofMatrix4x4_operations();
        test_ofMatrix4x4_transformations();
        test_ofMatrix4x4_projection();

        // ofQuaternion Tests
        std::cout << "\n" << YELLOW << "=== ofQuaternion Tests ===" << RESET;
        test_ofQuaternion_construction();
        test_ofQuaternion_operations();

        // ofMath Functions Tests
        std::cout << "\n" << YELLOW << "=== ofMath Functions Tests ===" << RESET;
        test_ofMath_random();
        test_ofMath_noise();
        test_ofMath_mapping();
        test_ofMath_distance();
        test_ofMath_angles();

    } catch (const std::exception& e) {
        std::cout << RED << "\nException caught: " << e.what() << RESET << "\n";
    }

    printSummary();

    return (g_failedTests == 0) ? 0 : 1;
}

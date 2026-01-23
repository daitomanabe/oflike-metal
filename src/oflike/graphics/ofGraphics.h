#pragma once

#include <cstdint>

// ============================================================================
// ofGraphics - openFrameworks Compatible Graphics API
// ============================================================================

/**
 * Graphics state management and drawing functions.
 *
 * This module provides oF-compatible drawing state functions including:
 * - Color management (ofSetColor, ofSetBackgroundColor)
 * - Background clearing (ofBackground, ofClear)
 * - Fill/stroke mode (ofFill, ofNoFill)
 * - Line width and shape resolution
 *
 * All functions are thread-safe when called from the main thread only.
 */

// ============================================================================
// Color Management
// ============================================================================

/**
 * Set the current drawing color (0-255 range).
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0-255), default: 255
 */
void ofSetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

/**
 * Set the current drawing color (0-255 range, grayscale).
 * @param gray Grayscale value (0-255)
 * @param a Alpha component (0-255), default: 255
 */
void ofSetColor(uint8_t gray, uint8_t a = 255);

/**
 * Set the current drawing color from hex (0xRRGGBB or 0xAARRGGBB).
 * @param hex Color as hexadecimal (0xRRGGBB or 0xAARRGGBB)
 * @param a Alpha component (0-255), default: 255 (ignored if hex includes alpha)
 */
void ofSetHexColor(uint32_t hex, uint8_t a = 255);

/**
 * Set the background color (0-255 range).
 * Does not clear immediately, use ofBackground() to clear.
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0-255), default: 255
 */
void ofSetBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

/**
 * Set the background color (0-255 range, grayscale).
 * @param gray Grayscale value (0-255)
 * @param a Alpha component (0-255), default: 255
 */
void ofSetBackgroundColor(uint8_t gray, uint8_t a = 255);

/**
 * Set the background color from hex.
 * @param hex Color as hexadecimal (0xRRGGBB or 0xAARRGGBB)
 * @param a Alpha component (0-255), default: 255
 */
void ofSetBackgroundHexColor(uint32_t hex, uint8_t a = 255);

// ============================================================================
// Background Clearing
// ============================================================================

/**
 * Clear the screen with current background color.
 */
void ofBackground();

/**
 * Clear the screen with specified color (0-255 range).
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0-255), default: 255
 */
void ofBackground(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

/**
 * Clear the screen with specified color (0-255 range, grayscale).
 * @param gray Grayscale value (0-255)
 * @param a Alpha component (0-255), default: 255
 */
void ofBackground(uint8_t gray, uint8_t a = 255);

/**
 * Clear the screen with specified color from hex.
 * @param hex Color as hexadecimal (0xRRGGBB or 0xAARRGGBB)
 * @param a Alpha component (0-255), default: 255
 */
void ofBackgroundHex(uint32_t hex, uint8_t a = 255);

/**
 * Clear color, depth, and/or stencil buffers.
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0-255)
 */
void ofClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

/**
 * Clear with current background color.
 */
void ofClear();

/**
 * Clear the depth buffer only.
 */
void ofClearDepth();

/**
 * Clear the alpha channel.
 */
void ofClearAlpha();

// ============================================================================
// Fill & Stroke Mode
// ============================================================================

/**
 * Enable fill mode (shapes are filled).
 * This is the default mode.
 */
void ofFill();

/**
 * Disable fill mode (shapes are outlined only).
 */
void ofNoFill();

/**
 * Check if fill mode is enabled.
 * @return true if fill mode is enabled, false otherwise
 */
bool ofGetFill();

// ============================================================================
// Line & Shape Settings
// ============================================================================

/**
 * Set the line width for stroke drawing.
 * @param width Line width in pixels (default: 1.0)
 */
void ofSetLineWidth(float width);

/**
 * Get the current line width.
 * @return Current line width in pixels
 */
float ofGetLineWidth();

/**
 * Set the circle resolution (number of segments).
 * Higher values = smoother circles, but slower rendering.
 * @param resolution Number of segments (default: 32)
 */
void ofSetCircleResolution(uint32_t resolution);

/**
 * Get the current circle resolution.
 * @return Number of segments used for circles
 */
uint32_t ofGetCircleResolution();

/**
 * Set the curve resolution (number of segments per curve).
 * @param resolution Number of segments (default: 20)
 */
void ofSetCurveResolution(uint32_t resolution);

/**
 * Get the current curve resolution.
 * @return Number of segments used for curves
 */
uint32_t ofGetCurveResolution();

/**
 * Set the sphere resolution (lat/lon segments).
 * @param resolution Number of segments (default: 20)
 */
void ofSetSphereResolution(uint32_t resolution);

/**
 * Get the current sphere resolution.
 * @return Number of segments used for spheres
 */
uint32_t ofGetSphereResolution();

// ============================================================================
// Drawing State Query
// ============================================================================

/**
 * Get the current draw color as RGBA (0-255 range).
 * @param r Output: Red component
 * @param g Output: Green component
 * @param b Output: Blue component
 * @param a Output: Alpha component
 */
void ofGetColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);

/**
 * Get the current background color as RGBA (0-255 range).
 * @param r Output: Red component
 * @param g Output: Green component
 * @param b Output: Blue component
 * @param a Output: Alpha component
 */
void ofGetBackgroundColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);

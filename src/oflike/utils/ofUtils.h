#pragma once

#include <string>

/// openFrameworks-compatible utility functions
/// Provides time, system, and other utility functions
///
/// Phase 2.4: Time functions
/// - ofGetElapsedTimef() - elapsed time in seconds
/// - ofGetElapsedTimeMillis() - elapsed time in milliseconds
/// - ofGetFrameNum() - current frame number
/// - ofGetFrameRate() - current frame rate (FPS)
/// - ofSetFrameRate() - set target frame rate

// MARK: - Time Functions

/// Get elapsed time since app started (seconds, float precision)
/// @return Elapsed time in seconds
float ofGetElapsedTimef();

/// Get elapsed time since app started (milliseconds)
/// @return Elapsed time in milliseconds
unsigned long long ofGetElapsedTimeMillis();

/// Get the number of frames rendered since app started
/// @return Current frame number
unsigned long long ofGetFrameNum();

/// Get current frame rate (frames per second)
/// @return Current FPS
float ofGetFrameRate();

/// Set target frame rate
/// @param targetRate Target frames per second (0 = unlimited)
void ofSetFrameRate(float targetRate);

// MARK: - Image Loading Functions

// Forward declarations
namespace oflike {
    template<typename PixelType> class ofPixels_;
    using ofPixels = ofPixels_<uint8_t>;
    using ofShortPixels = ofPixels_<uint16_t>;
    using ofFloatPixels = ofPixels_<float>;
    class ofTexture;
}

using oflike::ofPixels;
using oflike::ofShortPixels;
using oflike::ofFloatPixels;
using oflike::ofTexture;

/// Load image from file into ofPixels
/// Uses ImageIO framework (CGImageSource) for loading
/// Supports PNG, JPG, GIF, BMP, TIFF, and more
/// @param pixels Target ofPixels object to load into
/// @param path Path to image file (absolute or relative)
/// @return true if successful, false otherwise
bool ofLoadImage(ofPixels& pixels, const std::string& path);

/// Load image from file into ofPixels (16-bit)
/// @param pixels Target ofShortPixels object to load into
/// @param path Path to image file
/// @return true if successful, false otherwise
bool ofLoadImage(ofShortPixels& pixels, const std::string& path);

/// Load image from file into ofPixels (float)
/// @param pixels Target ofFloatPixels object to load into
/// @param path Path to image file
/// @return true if successful, false otherwise
bool ofLoadImage(ofFloatPixels& pixels, const std::string& path);

/// Load image from file directly into ofTexture (GPU)
/// Uses MTKTextureLoader for direct GPU upload (fastest path)
/// Falls back to ImageIO if MTKTextureLoader fails
/// Preferred method for images that will only be rendered
/// @param texture Target ofTexture object to load into
/// @param path Path to image file
/// @return true if successful, false otherwise
bool ofLoadImage(ofTexture& texture, const std::string& path);

// MARK: - Image Saving Functions

/// Save ofPixels to image file
/// Uses CGImageDestination for saving
/// Supported formats: PNG, JPG, TIFF (determined by file extension)
/// @param pixels Source ofPixels object to save
/// @param path Path to save image file (extension determines format)
/// @param quality JPEG quality 0.0-1.0 (default 0.9), ignored for PNG/TIFF
/// @return true if successful, false otherwise
bool ofSaveImage(const ofPixels& pixels, const std::string& path, float quality = 0.9f);

/// Save ofShortPixels to image file (16-bit)
/// @param pixels Source ofShortPixels object to save
/// @param path Path to save image file
/// @param quality JPEG quality 0.0-1.0, ignored for PNG/TIFF
/// @return true if successful, false otherwise
bool ofSaveImage(const ofShortPixels& pixels, const std::string& path, float quality = 0.9f);

/// Save ofFloatPixels to image file (float)
/// @param pixels Source ofFloatPixels object to save
/// @param path Path to save image file
/// @param quality JPEG quality 0.0-1.0, ignored for PNG/TIFF
/// @return true if successful, false otherwise
bool ofSaveImage(const ofFloatPixels& pixels, const std::string& path, float quality = 0.9f);

// MARK: - Keyboard State Functions

/// Check if a specific key is currently pressed
/// @param key Key character or key code to check
/// @return true if the key is currently pressed, false otherwise
bool ofGetKeyPressed(int key);

// MARK: - Mouse State Functions (Phase 13.5)

/// Get current mouse x position (in pixels)
/// @return Current mouse x position
int ofGetMouseX();

/// Get current mouse y position (in pixels)
/// @return Current mouse y position
int ofGetMouseY();

/// Get previous mouse x position before last event (in pixels)
/// @return Previous mouse x position
int ofGetPreviousMouseX();

/// Get previous mouse y position before last event (in pixels)
/// @return Previous mouse y position
int ofGetPreviousMouseY();

/// Check if a mouse button is currently pressed
/// @param button Mouse button index (0=left, 1=right, 2=middle)
/// @return true if the button is pressed, false otherwise
bool ofGetMousePressed(int button = 0);

// MARK: - Window Functions (Phase 14.1)

/// Get window width in pixels
/// Alias for ofGetWindowWidth()
/// @return Current window width
int ofGetWidth();

/// Get window height in pixels
/// Alias for ofGetWindowHeight()
/// @return Current window height
int ofGetHeight();

/// Get window width in pixels
/// @return Current window width
int ofGetWindowWidth();

/// Get window height in pixels
/// @return Current window height
int ofGetWindowHeight();

/// Set window size
/// @param width New window width in pixels
/// @param height New window height in pixels
void ofSetWindowShape(int width, int height);

/// Set window position on screen
/// @param x New window x position in screen coordinates
/// @param y New window y position in screen coordinates
void ofSetWindowPosition(int x, int y);

/// Set window title
/// @param title New window title string
void ofSetWindowTitle(const std::string& title);

/// Set fullscreen mode
/// @param fullscreen true to enter fullscreen, false to exit fullscreen
void ofSetFullscreen(bool fullscreen);

#pragma once

// oflike-metal ofVideoGrabber - openFrameworks API compatible camera input
// Provides camera capture using AVFoundation and Metal textures

#include <string>
#include <memory>
#include <vector>
#include "../image/ofTexture.h"
#include "../image/ofPixels.h"

namespace oflike {

/// \brief Camera device information
struct ofVideoDevice {
    int id = -1;
    std::string name;
    std::string uniqueID;
    bool isDefault = false;
};

/// \brief Camera capture with AVFoundation backend
/// \details ofVideoGrabber provides openFrameworks-compatible camera capture
/// using AVCaptureSession for input and Metal textures for rendering.
///
/// Features:
/// - List available camera devices
/// - Capture from built-in or external cameras
/// - Request specific resolution and frame rate
/// - Access frames as textures or pixels
///
/// Implementation:
/// - Uses AVCaptureSession for capture
/// - Uses AVCaptureVideoDataOutput for frame access
/// - Uses Metal textures for GPU rendering
/// - pImpl pattern to hide Objective-C++ details
///
/// Example:
/// \code
///     ofVideoGrabber grabber;
///     grabber.setDeviceID(0);
///     grabber.setup(1280, 720);
///
///     // In update()
///     grabber.update();
///
///     // In draw()
///     grabber.draw(0, 0);
/// \endcode
class ofVideoGrabber {
public:
    // ========================================================================
    // Constructors & Destructor
    // ========================================================================

    /// \brief Default constructor
    ofVideoGrabber();

    /// \brief Destructor
    ~ofVideoGrabber();

    /// \brief Move constructor
    ofVideoGrabber(ofVideoGrabber&& other) noexcept;

    /// \brief Move assignment
    ofVideoGrabber& operator=(ofVideoGrabber&& other) noexcept;

    // Delete copy constructor and assignment
    ofVideoGrabber(const ofVideoGrabber&) = delete;
    ofVideoGrabber& operator=(const ofVideoGrabber&) = delete;

    // ========================================================================
    // Device Selection
    // ========================================================================

    /// \brief Get list of available video devices
    /// \return Vector of available camera devices
    static std::vector<ofVideoDevice> listDevices();

    /// \brief Set device to use by ID
    /// \param deviceID Device ID (index from listDevices)
    void setDeviceID(int deviceID);

    /// \brief Set device to use by unique ID string
    /// \param uniqueID Unique device identifier
    void setDeviceID(const std::string& uniqueID);

    /// \brief Get current device ID
    /// \return Current device ID
    int getDeviceID() const;

    // ========================================================================
    // Setup & Configuration
    // ========================================================================

    /// \brief Initialize camera with requested settings
    /// \param w Requested width (may be adjusted to closest supported)
    /// \param h Requested height (may be adjusted to closest supported)
    /// \return true if successful, false otherwise
    bool setup(int w, int h);

    /// \brief Close camera and release resources
    void close();

    /// \brief Check if camera is initialized
    /// \return true if camera is initialized
    bool isInitialized() const;

    /// \brief Set desired frame rate
    /// \param framerate Desired frame rate (may be adjusted)
    void setDesiredFrameRate(float framerate);

    /// \brief Get actual frame rate
    /// \return Actual capture frame rate
    float getFrameRate() const;

    // ========================================================================
    // Frame Update
    // ========================================================================

    /// \brief Update camera frame
    /// \details Must be called every frame to get new camera data
    void update();

    /// \brief Check if new frame is available
    /// \return true if a new frame arrived since last update
    bool isFrameNew() const;

    // ========================================================================
    // Drawing
    // ========================================================================

    /// \brief Draw camera frame at position
    /// \param x X coordinate
    /// \param y Y coordinate
    void draw(float x, float y) const;

    /// \brief Draw camera frame at position with size
    /// \param x X coordinate
    /// \param y Y coordinate
    /// \param w Width
    /// \param h Height
    void draw(float x, float y, float w, float h) const;

    // ========================================================================
    // Texture & Pixel Access
    // ========================================================================

    /// \brief Get reference to camera texture
    /// \return Reference to ofTexture for custom rendering
    ofTexture& getTexture();

    /// \brief Get const reference to camera texture
    /// \return Const reference to ofTexture
    const ofTexture& getTexture() const;

    /// \brief Get pixel data from current frame
    /// \return Reference to ofPixels containing frame data
    ofPixels& getPixels();

    /// \brief Get const pixel data from current frame
    /// \return Const reference to ofPixels
    const ofPixels& getPixels() const;

    // ========================================================================
    // Camera Properties
    // ========================================================================

    /// \brief Get capture width
    /// \return Width in pixels
    float getWidth() const;

    /// \brief Get capture height
    /// \return Height in pixels
    float getHeight() const;

    // ========================================================================
    // Permissions
    // ========================================================================

    /// \brief Check if camera access is authorized
    /// \return true if authorized, false otherwise
    static bool isAuthorized();

    /// \brief Request camera access permission
    /// \details Opens system dialog to request camera access.
    /// Returns immediately; use isAuthorized() to check result.
    static void requestPermission();

private:
    // ========================================================================
    // pImpl Pattern
    // ========================================================================

    struct Impl;
    std::unique_ptr<Impl> impl_;

    // ========================================================================
    // Internal Helper Methods
    // ========================================================================

    /// \brief Initialize impl if needed
    void ensureImpl();
};

} // namespace oflike

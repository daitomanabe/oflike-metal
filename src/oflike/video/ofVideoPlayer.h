#pragma once

// oflike-metal ofVideoPlayer - openFrameworks API compatible video player
// Provides video playback using AVFoundation and Metal textures

#include <string>
#include <memory>
#include "../image/ofTexture.h"
#include "../image/ofPixels.h"

namespace oflike {

/// \brief Video loop type
enum ofLoopType {
    OF_LOOP_NONE = 0,
    OF_LOOP_NORMAL = 1,
    OF_LOOP_PALINDROME = 2
};

/// \brief Video playback state
enum ofVideoState {
    OF_VIDEO_STATE_STOPPED = 0,
    OF_VIDEO_STATE_PLAYING = 1,
    OF_VIDEO_STATE_PAUSED = 2
};

/// \brief Video player with AVFoundation backend
/// \details ofVideoPlayer provides openFrameworks-compatible video playback
/// using AVFoundation for decoding and Metal textures for rendering.
///
/// Features:
/// - Load video files (MP4, MOV, M4V, AVI, etc. via AVFoundation)
/// - Play, pause, stop, and seek
/// - Loop modes (none, normal, palindrome)
/// - Frame-by-frame access
/// - Audio control (volume, mute)
/// - Speed control
///
/// Implementation:
/// - Uses AVPlayer for playback
/// - Uses AVPlayerItemVideoOutput for frame access
/// - Uses Metal textures for GPU rendering
/// - pImpl pattern to hide Objective-C++ details
///
/// Example:
/// \code
///     ofVideoPlayer video;
///     video.load("movie.mp4");
///     video.play();
///
///     // In update()
///     video.update();
///
///     // In draw()
///     video.draw(0, 0);
/// \endcode
class ofVideoPlayer {
public:
    // ========================================================================
    // Constructors & Destructor
    // ========================================================================

    /// \brief Default constructor
    ofVideoPlayer();

    /// \brief Destructor
    ~ofVideoPlayer();

    /// \brief Move constructor
    ofVideoPlayer(ofVideoPlayer&& other) noexcept;

    /// \brief Move assignment
    ofVideoPlayer& operator=(ofVideoPlayer&& other) noexcept;

    // Delete copy constructor and assignment
    ofVideoPlayer(const ofVideoPlayer&) = delete;
    ofVideoPlayer& operator=(const ofVideoPlayer&) = delete;

    // ========================================================================
    // Loading
    // ========================================================================

    /// \brief Load video from file
    /// \param path Path to video file (relative or absolute)
    /// \return true if successful, false otherwise
    bool load(const std::string& path);

    /// \brief Close video and release resources
    void close();

    /// \brief Check if video is loaded
    /// \return true if video is loaded, false otherwise
    bool isLoaded() const;

    // ========================================================================
    // Playback Control
    // ========================================================================

    /// \brief Start playback
    void play();

    /// \brief Pause playback
    void pause();

    /// \brief Stop playback (reset to beginning)
    void stop();

    /// \brief Toggle pause state
    void setPaused(bool paused);

    /// \brief Check if playing
    /// \return true if playing
    bool isPlaying() const;

    /// \brief Check if paused
    /// \return true if paused
    bool isPaused() const;

    /// \brief Check if video has finished
    /// \return true if playback reached end (and not looping)
    bool isFinished() const;

    // ========================================================================
    // Looping
    // ========================================================================

    /// \brief Set loop mode
    /// \param loopType OF_LOOP_NONE, OF_LOOP_NORMAL, or OF_LOOP_PALINDROME
    void setLoopState(ofLoopType loopType);

    /// \brief Get current loop mode
    /// \return Current loop type
    ofLoopType getLoopState() const;

    // ========================================================================
    // Position & Seeking
    // ========================================================================

    /// \brief Get current playback position
    /// \return Position in seconds
    float getPosition() const;

    /// \brief Set playback position (0.0 to 1.0)
    /// \param pct Position as percentage (0.0 = start, 1.0 = end)
    void setPosition(float pct);

    /// \brief Get current time in seconds
    /// \return Current time in seconds
    float getCurrentTime() const;

    /// \brief Seek to specific time
    /// \param seconds Time in seconds
    void setTime(float seconds);

    /// \brief Get current frame number
    /// \return Current frame number
    int getCurrentFrame() const;

    /// \brief Seek to specific frame
    /// \param frame Frame number
    void setFrame(int frame);

    /// \brief Seek forward by specified seconds
    void seekForward(float seconds);

    /// \brief Seek backward by specified seconds
    void seekBackward(float seconds);

    /// \brief Go to first frame
    void firstFrame();

    /// \brief Advance to next frame
    void nextFrame();

    /// \brief Go back to previous frame
    void previousFrame();

    // ========================================================================
    // Speed Control
    // ========================================================================

    /// \brief Get playback speed
    /// \return Playback speed (1.0 = normal)
    float getSpeed() const;

    /// \brief Set playback speed
    /// \param speed Playback speed (1.0 = normal, 2.0 = double, etc.)
    void setSpeed(float speed);

    // ========================================================================
    // Audio Control
    // ========================================================================

    /// \brief Get volume level
    /// \return Volume (0.0 to 1.0)
    float getVolume() const;

    /// \brief Set volume level
    /// \param volume Volume (0.0 to 1.0)
    void setVolume(float volume);

    // ========================================================================
    // Frame Update
    // ========================================================================

    /// \brief Update video frame
    /// \details Must be called every frame to update the texture
    void update();

    /// \brief Check if new frame is available
    /// \return true if a new frame was decoded since last update
    bool isFrameNew() const;

    // ========================================================================
    // Drawing
    // ========================================================================

    /// \brief Draw video at position
    /// \param x X coordinate
    /// \param y Y coordinate
    void draw(float x, float y) const;

    /// \brief Draw video at position with size
    /// \param x X coordinate
    /// \param y Y coordinate
    /// \param w Width
    /// \param h Height
    void draw(float x, float y, float w, float h) const;

    // ========================================================================
    // Texture & Pixel Access
    // ========================================================================

    /// \brief Get reference to video texture
    /// \return Reference to ofTexture for custom rendering
    ofTexture& getTexture();

    /// \brief Get const reference to video texture
    /// \return Const reference to ofTexture
    const ofTexture& getTexture() const;

    /// \brief Get pixel data from current frame
    /// \return Reference to ofPixels containing frame data
    ofPixels& getPixels();

    /// \brief Get const pixel data from current frame
    /// \return Const reference to ofPixels
    const ofPixels& getPixels() const;

    // ========================================================================
    // Video Properties
    // ========================================================================

    /// \brief Get video width
    /// \return Width in pixels
    float getWidth() const;

    /// \brief Get video height
    /// \return Height in pixels
    float getHeight() const;

    /// \brief Get total duration
    /// \return Duration in seconds
    float getDuration() const;

    /// \brief Get total number of frames
    /// \return Total frame count
    int getTotalNumFrames() const;

    /// \brief Get video frame rate
    /// \return Frame rate in frames per second
    float getFrameRate() const;

    /// \brief Check if video has audio
    /// \return true if video contains audio track
    bool hasAudio() const;

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

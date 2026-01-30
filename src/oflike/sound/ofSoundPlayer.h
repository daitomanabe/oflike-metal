#pragma once

#include <string>
#include <memory>

namespace oflike {

/// Loop states for sound playback
enum ofLoopType {
    OF_LOOP_NONE = 0,
    OF_LOOP_NORMAL = 1,
    OF_LOOP_PALINDROME = 2  // Not supported for audio, treated as LOOP_NORMAL
};

/// Sound player for audio file playback using AVFoundation
class ofSoundPlayer {
public:
    ofSoundPlayer();
    ~ofSoundPlayer();

    // Move semantics
    ofSoundPlayer(ofSoundPlayer&& other) noexcept;
    ofSoundPlayer& operator=(ofSoundPlayer&& other) noexcept;

    // Disable copy
    ofSoundPlayer(const ofSoundPlayer&) = delete;
    ofSoundPlayer& operator=(const ofSoundPlayer&) = delete;

    // ========================================
    // Loading
    // ========================================

    /// Load a sound file
    /// @param path Path to the audio file (relative to app bundle or absolute)
    /// @param stream If true, stream the audio instead of loading entirely into memory
    /// @return true if loaded successfully
    bool load(const std::string& path, bool stream = false);

    /// Unload the sound and release resources
    void unload();

    /// Check if a sound is loaded
    bool isLoaded() const;

    // ========================================
    // Playback Control
    // ========================================

    /// Start playing the sound
    void play();

    /// Stop playing
    void stop();

    /// Pause playback
    void pause();

    /// Set whether the sound is paused
    void setPaused(bool paused);

    /// Check if the sound is currently playing
    bool isPlaying() const;

    // ========================================
    // Looping
    // ========================================

    /// Set the loop state
    void setLoop(bool loop);

    /// Get whether looping is enabled
    bool isLooping() const;

    // ========================================
    // Volume & Pan
    // ========================================

    /// Set the volume (0.0 - 1.0)
    void setVolume(float volume);

    /// Get the current volume
    float getVolume() const;

    /// Set the pan (-1.0 = left, 0.0 = center, 1.0 = right)
    void setPan(float pan);

    /// Get the current pan
    float getPan() const;

    // ========================================
    // Speed & Pitch
    // ========================================

    /// Set the playback speed (1.0 = normal, 0.5 = half speed, 2.0 = double speed)
    void setSpeed(float speed);

    /// Get the current playback speed
    float getSpeed() const;

    // ========================================
    // Position
    // ========================================

    /// Set the playback position (0.0 - 1.0)
    void setPosition(float pct);

    /// Get the current playback position (0.0 - 1.0)
    float getPosition() const;

    /// Set the playback position in milliseconds
    void setPositionMS(int ms);

    /// Get the current playback position in milliseconds
    int getPositionMS() const;

    // ========================================
    // Multi-play (for sound effects)
    // ========================================

    /// Enable multi-play mode (allows overlapping plays)
    void setMultiPlay(bool multiPlay);

    /// Check if multi-play is enabled
    bool isMultiPlay() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    void ensureImpl();
};

// ========================================
// Global Sound Functions
// ========================================

/// Set the global volume for all sounds (0.0 - 1.0)
void ofSoundSetVolume(float volume);

/// Stop all currently playing sounds
void ofSoundStopAll();

/// Update the sound system (call once per frame)
void ofSoundUpdate();

} // namespace oflike

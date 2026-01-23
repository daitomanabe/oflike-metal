#pragma once

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

#pragma once

// ofxOscParameterSync - Bidirectional OSC parameter synchronization
// Automatically syncs ofParameter values via OSC

#include "ofxOscSender.h"
#include "ofxOscReceiver.h"
#include "../../../oflike/types/ofParameter.h"
#include <memory>
#include <string>

/// Bidirectional OSC parameter synchronization
class ofxOscParameterSync {
public:
    ofxOscParameterSync();
    ~ofxOscParameterSync();

    /// Setup with send and receive ports
    /// @param sendHost Host to send OSC messages to
    /// @param sendPort Port to send OSC messages to
    /// @param receivePort Port to receive OSC messages on
    void setup(const std::string& sendHost, int sendPort, int receivePort);

    /// Setup sender only (one-way sync)
    void setupSender(const std::string& sendHost, int sendPort);

    /// Setup receiver only (one-way sync)
    void setupReceiver(int receivePort);

    /// Add parameter to sync
    /// @param param Parameter to synchronize
    /// @param oscAddress OSC address (e.g., "/params/volume")
    template<typename T>
    void add(ofParameter<T>& param, const std::string& oscAddress);

    /// Add parameter group to sync
    /// @param group Parameter group to synchronize
    /// @param oscAddressPrefix OSC address prefix (e.g., "/params")
    void add(ofParameterGroup& group, const std::string& oscAddressPrefix);

    /// Update - call this in your update() loop
    /// Processes incoming OSC messages and sends outgoing changes
    void update();

    /// Send all parameter values immediately
    void sendAll();

    /// Enable/disable automatic sending on parameter change
    void setAutoSend(bool autoSend);

    /// Enable/disable automatic receiving
    void setAutoReceive(bool autoReceive);

    /// Clear all synced parameters
    void clear();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

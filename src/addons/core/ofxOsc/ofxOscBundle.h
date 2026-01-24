#pragma once

#include "ofxOscMessage.h"
#include <vector>
#include <cstdint>

/// \brief Represents an OSC bundle containing multiple messages and/or nested bundles.
///
/// ofxOscBundle allows you to group multiple OSC messages together with a time tag.
/// Bundles can also contain other bundles, allowing for hierarchical message organization.
/// According to the OSC specification, all messages in a bundle should be processed
/// at the time indicated by the bundle's time tag.
///
/// Example usage:
/// \code
/// ofxOscBundle bundle;
/// bundle.setTimeTag(getCurrentTime());
///
/// ofxOscMessage msg1;
/// msg1.setAddress("/synth/note");
/// msg1.addIntArg(60);
/// bundle.addMessage(msg1);
///
/// ofxOscMessage msg2;
/// msg2.setAddress("/synth/freq");
/// msg2.addFloatArg(440.0f);
/// bundle.addMessage(msg2);
/// \endcode
class ofxOscBundle {
public:
    ofxOscBundle();
    ~ofxOscBundle();

    // Copy constructor and assignment
    ofxOscBundle(const ofxOscBundle& other);
    ofxOscBundle& operator=(const ofxOscBundle& other);

    // Time tag
    /// \brief Set the OSC time tag (NTP timestamp)
    /// \param timeTag 64-bit NTP timestamp (0x0000000000000001 = immediate)
    void setTimeTag(uint64_t timeTag);

    /// \brief Get the OSC time tag
    uint64_t getTimeTag() const;

    // Add messages
    /// \brief Add an OSC message to the bundle
    void addMessage(const ofxOscMessage& message);

    /// \brief Get the number of messages in the bundle
    size_t getMessageCount() const;

    /// \brief Get the message at the given index
    /// \param index Message index (0-based)
    /// \return Reference to the message
    const ofxOscMessage& getMessageAt(size_t index) const;

    // Add nested bundles
    /// \brief Add a nested bundle to this bundle
    void addBundle(const ofxOscBundle& bundle);

    /// \brief Get the number of nested bundles
    size_t getBundleCount() const;

    /// \brief Get the nested bundle at the given index
    /// \param index Bundle index (0-based)
    /// \return Reference to the nested bundle
    const ofxOscBundle& getBundleAt(size_t index) const;

    // Clear
    /// \brief Clear all messages and nested bundles
    void clear();

    // Utility
    /// \brief Check if the bundle is empty (no messages or nested bundles)
    bool isEmpty() const;

    /// \brief Get the total number of items (messages + bundles)
    size_t getTotalItemCount() const;

    // Internal: Access raw data for serialization
    const std::vector<ofxOscMessage>& getMessages() const { return messages_; }
    const std::vector<ofxOscBundle>& getBundles() const { return bundles_; }

private:
    uint64_t timeTag_;                       // NTP timestamp (0x1 = immediate)
    std::vector<ofxOscMessage> messages_;    // Messages in this bundle
    std::vector<ofxOscBundle> bundles_;      // Nested bundles

    void checkMessageIndex(size_t index) const;
    void checkBundleIndex(size_t index) const;
};

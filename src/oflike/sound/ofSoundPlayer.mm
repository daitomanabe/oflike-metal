#import "ofSoundPlayer.h"
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import <vector>
#import <mutex>

namespace oflike {

// Global state for sound management
static float globalVolume = 1.0f;
static std::vector<ofSoundPlayer*> activePlayers;
static std::mutex playersMutex;

// ============================================================================
// ofSoundPlayer::Impl
// ============================================================================

struct ofSoundPlayer::Impl {
    AVAudioPlayer* player = nil;
    NSMutableArray<AVAudioPlayer*>* multiPlayers = nil;  // For multi-play mode

    std::string filePath;
    bool loaded = false;
    bool multiPlay = false;
    float volume = 1.0f;
    float pan = 0.0f;
    float speed = 1.0f;
    bool looping = false;

    Impl() {
        multiPlayers = [[NSMutableArray alloc] init];
    }

    ~Impl() {
        unload();
    }

    void unload() {
        @autoreleasepool {
            if (player) {
                [player stop];
                player = nil;
            }

            for (AVAudioPlayer* p in multiPlayers) {
                [p stop];
            }
            [multiPlayers removeAllObjects];

            loaded = false;
            filePath.clear();
        }
    }

    bool load(const std::string& path, bool stream) {
        @autoreleasepool {
            unload();

            // Resolve path
            NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
            NSURL* url = nil;

            // Check if it's a URL
            if ([nsPath hasPrefix:@"http://"] || [nsPath hasPrefix:@"https://"]) {
                url = [NSURL URLWithString:nsPath];
            } else {
                // Handle relative paths
                if (![nsPath hasPrefix:@"/"]) {
                    NSString* bundlePath = [[NSBundle mainBundle] resourcePath];
                    nsPath = [bundlePath stringByAppendingPathComponent:nsPath];
                }
                url = [NSURL fileURLWithPath:nsPath];
            }

            if (!url) {
                NSLog(@"ofSoundPlayer: Invalid path: %s", path.c_str());
                return false;
            }

            // Check if file exists
            if ([url isFileURL] && ![[NSFileManager defaultManager] fileExistsAtPath:[url path]]) {
                NSLog(@"ofSoundPlayer: File not found: %@", [url path]);
                return false;
            }

            // Create player
            NSError* error = nil;
            player = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];

            if (error || !player) {
                NSLog(@"ofSoundPlayer: Failed to load: %@ - %@",
                      [url path], error ? [error localizedDescription] : @"Unknown error");
                return false;
            }

            // Prepare for playback
            [player prepareToPlay];

            // Apply current settings
            player.volume = volume * globalVolume;
            player.pan = pan;
            player.enableRate = YES;
            player.rate = speed;
            player.numberOfLoops = looping ? -1 : 0;

            filePath = path;
            loaded = true;

            NSLog(@"ofSoundPlayer: Loaded %s (%.2f sec)", path.c_str(), player.duration);
            return true;
        }
    }

    void play() {
        if (!loaded || !player) return;

        @autoreleasepool {
            if (multiPlay) {
                // Create a new player instance for overlapping playback
                NSError* error = nil;
                AVAudioPlayer* newPlayer = [[AVAudioPlayer alloc]
                    initWithContentsOfURL:[NSURL fileURLWithPath:
                        [NSString stringWithUTF8String:filePath.c_str()]]
                    error:&error];

                if (newPlayer) {
                    newPlayer.volume = volume * globalVolume;
                    newPlayer.pan = pan;
                    newPlayer.enableRate = YES;
                    newPlayer.rate = speed;
                    newPlayer.numberOfLoops = 0;  // Multi-play sounds don't loop
                    [newPlayer prepareToPlay];
                    [newPlayer play];
                    [multiPlayers addObject:newPlayer];

                    // Clean up finished players
                    NSMutableArray* toRemove = [[NSMutableArray alloc] init];
                    for (AVAudioPlayer* p in multiPlayers) {
                        if (!p.isPlaying) {
                            [toRemove addObject:p];
                        }
                    }
                    [multiPlayers removeObjectsInArray:toRemove];
                }
            } else {
                // Normal single-play mode
                player.currentTime = 0;
                [player play];
            }
        }
    }

    void stop() {
        @autoreleasepool {
            if (player) {
                [player stop];
                player.currentTime = 0;
            }

            for (AVAudioPlayer* p in multiPlayers) {
                [p stop];
            }
            [multiPlayers removeAllObjects];
        }
    }

    void pause() {
        @autoreleasepool {
            if (player) {
                [player pause];
            }
        }
    }

    bool isPlaying() const {
        if (player && player.isPlaying) return true;

        for (AVAudioPlayer* p in multiPlayers) {
            if (p.isPlaying) return true;
        }
        return false;
    }

    void setVolume(float vol) {
        volume = std::max(0.0f, std::min(1.0f, vol));
        @autoreleasepool {
            if (player) {
                player.volume = volume * globalVolume;
            }
            for (AVAudioPlayer* p in multiPlayers) {
                p.volume = volume * globalVolume;
            }
        }
    }

    void setPan(float p) {
        pan = std::max(-1.0f, std::min(1.0f, p));
        @autoreleasepool {
            if (player) {
                player.pan = pan;
            }
            for (AVAudioPlayer* pl in multiPlayers) {
                pl.pan = pan;
            }
        }
    }

    void setSpeed(float s) {
        speed = std::max(0.5f, std::min(2.0f, s));  // AVAudioPlayer range
        @autoreleasepool {
            if (player) {
                player.rate = speed;
            }
            for (AVAudioPlayer* p in multiPlayers) {
                p.rate = speed;
            }
        }
    }

    void setLoop(bool loop) {
        looping = loop;
        @autoreleasepool {
            if (player) {
                player.numberOfLoops = looping ? -1 : 0;
            }
        }
    }

    void setPosition(float pct) {
        pct = std::max(0.0f, std::min(1.0f, pct));
        @autoreleasepool {
            if (player) {
                player.currentTime = pct * player.duration;
            }
        }
    }

    float getPosition() const {
        if (!player || player.duration <= 0) return 0.0f;
        return static_cast<float>(player.currentTime / player.duration);
    }

    void setPositionMS(int ms) {
        @autoreleasepool {
            if (player) {
                player.currentTime = std::max(0.0, std::min(static_cast<double>(ms) / 1000.0, player.duration));
            }
        }
    }

    int getPositionMS() const {
        if (!player) return 0;
        return static_cast<int>(player.currentTime * 1000.0);
    }
};

// ============================================================================
// ofSoundPlayer Implementation
// ============================================================================

ofSoundPlayer::ofSoundPlayer()
    : impl_(std::make_unique<Impl>()) {
    std::lock_guard<std::mutex> lock(playersMutex);
    activePlayers.push_back(this);
}

ofSoundPlayer::~ofSoundPlayer() {
    std::lock_guard<std::mutex> lock(playersMutex);
    activePlayers.erase(
        std::remove(activePlayers.begin(), activePlayers.end(), this),
        activePlayers.end()
    );
}

ofSoundPlayer::ofSoundPlayer(ofSoundPlayer&& other) noexcept = default;
ofSoundPlayer& ofSoundPlayer::operator=(ofSoundPlayer&& other) noexcept = default;

void ofSoundPlayer::ensureImpl() {
    if (!impl_) {
        impl_ = std::make_unique<Impl>();
    }
}

bool ofSoundPlayer::load(const std::string& path, bool stream) {
    ensureImpl();
    return impl_->load(path, stream);
}

void ofSoundPlayer::unload() {
    if (impl_) {
        impl_->unload();
    }
}

bool ofSoundPlayer::isLoaded() const {
    return impl_ && impl_->loaded;
}

void ofSoundPlayer::play() {
    if (impl_) impl_->play();
}

void ofSoundPlayer::stop() {
    if (impl_) impl_->stop();
}

void ofSoundPlayer::pause() {
    if (impl_) impl_->pause();
}

void ofSoundPlayer::setPaused(bool paused) {
    if (paused) {
        pause();
    } else {
        play();
    }
}

bool ofSoundPlayer::isPlaying() const {
    return impl_ && impl_->isPlaying();
}

void ofSoundPlayer::setLoop(bool loop) {
    if (impl_) impl_->setLoop(loop);
}

bool ofSoundPlayer::isLooping() const {
    return impl_ && impl_->looping;
}

void ofSoundPlayer::setVolume(float volume) {
    if (impl_) impl_->setVolume(volume);
}

float ofSoundPlayer::getVolume() const {
    return impl_ ? impl_->volume : 1.0f;
}

void ofSoundPlayer::setPan(float pan) {
    if (impl_) impl_->setPan(pan);
}

float ofSoundPlayer::getPan() const {
    return impl_ ? impl_->pan : 0.0f;
}

void ofSoundPlayer::setSpeed(float speed) {
    if (impl_) impl_->setSpeed(speed);
}

float ofSoundPlayer::getSpeed() const {
    return impl_ ? impl_->speed : 1.0f;
}

void ofSoundPlayer::setPosition(float pct) {
    if (impl_) impl_->setPosition(pct);
}

float ofSoundPlayer::getPosition() const {
    return impl_ ? impl_->getPosition() : 0.0f;
}

void ofSoundPlayer::setPositionMS(int ms) {
    if (impl_) impl_->setPositionMS(ms);
}

int ofSoundPlayer::getPositionMS() const {
    return impl_ ? impl_->getPositionMS() : 0;
}

void ofSoundPlayer::setMultiPlay(bool multiPlay) {
    if (impl_) impl_->multiPlay = multiPlay;
}

bool ofSoundPlayer::isMultiPlay() const {
    return impl_ && impl_->multiPlay;
}

// ============================================================================
// Global Functions
// ============================================================================

void ofSoundSetVolume(float volume) {
    globalVolume = std::max(0.0f, std::min(1.0f, volume));

    std::lock_guard<std::mutex> lock(playersMutex);
    for (ofSoundPlayer* player : activePlayers) {
        if (player->isLoaded()) {
            player->setVolume(player->getVolume());  // Trigger volume update
        }
    }
}

void ofSoundStopAll() {
    std::lock_guard<std::mutex> lock(playersMutex);
    for (ofSoundPlayer* player : activePlayers) {
        player->stop();
    }
}

void ofSoundUpdate() {
    // Currently no-op, but can be used for future features
    // like FFT analysis or automatic cleanup
}

} // namespace oflike

#import "ofVideoPlayer.h"
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import "../../core/Context.h"

namespace oflike {

// ============================================================================
// ofVideoPlayer::Impl
// ============================================================================

struct ofVideoPlayer::Impl {
    AVPlayer* player = nil;
    AVPlayerItem* playerItem = nil;
    AVPlayerItemVideoOutput* videoOutput = nil;
    AVAsset* asset = nil;

    ofTexture texture;
    ofPixels pixels;

    int width = 0;
    int height = 0;
    float duration = 0.0f;
    float frameRate = 30.0f;
    int totalFrames = 0;

    bool loaded = false;
    bool playing = false;
    bool paused = false;
    bool finished = false;
    bool frameNew = false;
    bool hasAudioTrack = false;

    ofLoopType loopType = OF_LOOP_NONE;
    float speed = 1.0f;
    float volume = 1.0f;

    // For palindrome loop
    bool playingForward = true;

    // KVO observer for playback end
    id endTimeObserver = nil;

    Impl() = default;

    ~Impl() {
        close();
    }

    void close() {
        @autoreleasepool {
            if (endTimeObserver) {
                [[NSNotificationCenter defaultCenter] removeObserver:endTimeObserver];
                endTimeObserver = nil;
            }

            if (player) {
                [player pause];
                player = nil;
            }

            playerItem = nil;
            videoOutput = nil;
            asset = nil;

            texture.clear();
            pixels.clear();

            width = 0;
            height = 0;
            duration = 0.0f;
            frameRate = 30.0f;
            totalFrames = 0;

            loaded = false;
            playing = false;
            paused = false;
            finished = false;
            frameNew = false;
            hasAudioTrack = false;
            playingForward = true;
        }
    }
};

// ============================================================================
// ofVideoPlayer Implementation
// ============================================================================

ofVideoPlayer::ofVideoPlayer()
    : impl_(std::make_unique<Impl>()) {
}

ofVideoPlayer::~ofVideoPlayer() = default;

ofVideoPlayer::ofVideoPlayer(ofVideoPlayer&& other) noexcept = default;

ofVideoPlayer& ofVideoPlayer::operator=(ofVideoPlayer&& other) noexcept = default;

void ofVideoPlayer::ensureImpl() {
    if (!impl_) {
        impl_ = std::make_unique<Impl>();
    }
}

// ============================================================================
// Loading
// ============================================================================

bool ofVideoPlayer::load(const std::string& path) {
    ensureImpl();

    // Close any existing video
    close();

    @autoreleasepool {
        // Create URL from path
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSURL* url = nil;

        // Check if it's a file path or URL
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
            NSLog(@"ofVideoPlayer: Invalid path: %s", path.c_str());
            return false;
        }

        // Check if file exists (for local files)
        if ([url isFileURL] && ![[NSFileManager defaultManager] fileExistsAtPath:[url path]]) {
            NSLog(@"ofVideoPlayer: File not found: %@", [url path]);
            return false;
        }

        // Create AVAsset
        impl_->asset = [AVAsset assetWithURL:url];
        if (!impl_->asset) {
            NSLog(@"ofVideoPlayer: Failed to create AVAsset");
            return false;
        }

        // Wait for asset to load
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        __block bool loadSuccess = false;

        [impl_->asset loadValuesAsynchronouslyForKeys:@[@"tracks", @"duration"]
                                    completionHandler:^{
            NSError* error = nil;
            AVKeyValueStatus tracksStatus = [impl_->asset statusOfValueForKey:@"tracks" error:&error];
            AVKeyValueStatus durationStatus = [impl_->asset statusOfValueForKey:@"duration" error:&error];

            loadSuccess = (tracksStatus == AVKeyValueStatusLoaded && durationStatus == AVKeyValueStatusLoaded);
            dispatch_semaphore_signal(semaphore);
        }];

        // Wait with timeout (5 seconds)
        dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC);
        if (dispatch_semaphore_wait(semaphore, timeout) != 0 || !loadSuccess) {
            NSLog(@"ofVideoPlayer: Failed to load asset tracks");
            impl_->asset = nil;
            return false;
        }

        // Get video track info
        NSArray* videoTracks = [impl_->asset tracksWithMediaType:AVMediaTypeVideo];
        if ([videoTracks count] == 0) {
            NSLog(@"ofVideoPlayer: No video track found");
            impl_->asset = nil;
            return false;
        }

        AVAssetTrack* videoTrack = [videoTracks firstObject];
        CGSize videoSize = [videoTrack naturalSize];
        CGAffineTransform preferredTransform = [videoTrack preferredTransform];

        // Account for video rotation
        if (CGAffineTransformIsIdentity(preferredTransform) == NO) {
            // Video might be rotated
            if (preferredTransform.a == 0 && preferredTransform.d == 0) {
                // 90 or 270 degree rotation
                std::swap(videoSize.width, videoSize.height);
            }
        }

        impl_->width = static_cast<int>(videoSize.width);
        impl_->height = static_cast<int>(videoSize.height);
        impl_->frameRate = [videoTrack nominalFrameRate];
        if (impl_->frameRate <= 0) {
            impl_->frameRate = 30.0f;  // Default fallback
        }

        // Get duration
        CMTime durationTime = [impl_->asset duration];
        impl_->duration = static_cast<float>(CMTimeGetSeconds(durationTime));
        impl_->totalFrames = static_cast<int>(impl_->duration * impl_->frameRate);

        // Check for audio track
        NSArray* audioTracks = [impl_->asset tracksWithMediaType:AVMediaTypeAudio];
        impl_->hasAudioTrack = [audioTracks count] > 0;

        // Create video output with pixel format
        NSDictionary* pixelBufferAttributes = @{
            (id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA),
            (id)kCVPixelBufferMetalCompatibilityKey: @YES
        };

        impl_->videoOutput = [[AVPlayerItemVideoOutput alloc]
            initWithPixelBufferAttributes:pixelBufferAttributes];

        if (!impl_->videoOutput) {
            NSLog(@"ofVideoPlayer: Failed to create video output");
            impl_->asset = nil;
            return false;
        }

        // Create player item
        impl_->playerItem = [AVPlayerItem playerItemWithAsset:impl_->asset];
        if (!impl_->playerItem) {
            NSLog(@"ofVideoPlayer: Failed to create player item");
            impl_->videoOutput = nil;
            impl_->asset = nil;
            return false;
        }

        [impl_->playerItem addOutput:impl_->videoOutput];

        // Create player
        impl_->player = [AVPlayer playerWithPlayerItem:impl_->playerItem];
        if (!impl_->player) {
            NSLog(@"ofVideoPlayer: Failed to create player");
            impl_->playerItem = nil;
            impl_->videoOutput = nil;
            impl_->asset = nil;
            return false;
        }

        // Set initial volume
        impl_->player.volume = impl_->volume;

        // Add end-of-playback observer
        __weak AVPlayer* weakPlayer = impl_->player;
        Impl* implPtr = impl_.get();

        impl_->endTimeObserver = [[NSNotificationCenter defaultCenter]
            addObserverForName:AVPlayerItemDidPlayToEndTimeNotification
            object:impl_->playerItem
            queue:[NSOperationQueue mainQueue]
            usingBlock:^(NSNotification* note) {
                if (implPtr->loopType == OF_LOOP_NONE) {
                    implPtr->finished = true;
                    implPtr->playing = false;
                } else if (implPtr->loopType == OF_LOOP_NORMAL) {
                    [weakPlayer seekToTime:kCMTimeZero];
                    [weakPlayer play];
                } else if (implPtr->loopType == OF_LOOP_PALINDROME) {
                    // Toggle direction
                    implPtr->playingForward = !implPtr->playingForward;
                    if (!implPtr->playingForward) {
                        // Play backwards (by setting negative rate)
                        weakPlayer.rate = -std::abs(implPtr->speed);
                    } else {
                        weakPlayer.rate = std::abs(implPtr->speed);
                    }
                }
            }];

        // Allocate texture and pixels
        impl_->texture.allocate(impl_->width, impl_->height, OF_IMAGE_COLOR_ALPHA);
        impl_->pixels.allocate(impl_->width, impl_->height, 4);

        impl_->loaded = true;
        impl_->finished = false;

        NSLog(@"ofVideoPlayer: Loaded video %dx%d @ %.2f fps, duration: %.2f sec",
              impl_->width, impl_->height, impl_->frameRate, impl_->duration);

        return true;
    }
}

void ofVideoPlayer::close() {
    if (impl_) {
        impl_->close();
    }
}

bool ofVideoPlayer::isLoaded() const {
    return impl_ && impl_->loaded;
}

// ============================================================================
// Playback Control
// ============================================================================

void ofVideoPlayer::play() {
    if (!impl_ || !impl_->loaded) return;

    @autoreleasepool {
        if (impl_->finished) {
            // Reset to beginning if finished
            [impl_->player seekToTime:kCMTimeZero];
            impl_->finished = false;
        }

        impl_->player.rate = impl_->playingForward ? impl_->speed : -impl_->speed;
        impl_->playing = true;
        impl_->paused = false;
    }
}

void ofVideoPlayer::pause() {
    if (!impl_ || !impl_->loaded) return;

    @autoreleasepool {
        [impl_->player pause];
        impl_->paused = true;
        impl_->playing = false;
    }
}

void ofVideoPlayer::stop() {
    if (!impl_ || !impl_->loaded) return;

    @autoreleasepool {
        [impl_->player pause];
        [impl_->player seekToTime:kCMTimeZero];
        impl_->playing = false;
        impl_->paused = false;
        impl_->finished = false;
        impl_->playingForward = true;
    }
}

void ofVideoPlayer::setPaused(bool paused) {
    if (paused) {
        pause();
    } else {
        play();
    }
}

bool ofVideoPlayer::isPlaying() const {
    return impl_ && impl_->playing;
}

bool ofVideoPlayer::isPaused() const {
    return impl_ && impl_->paused;
}

bool ofVideoPlayer::isFinished() const {
    return impl_ && impl_->finished;
}

// ============================================================================
// Looping
// ============================================================================

void ofVideoPlayer::setLoopState(ofLoopType loopType) {
    if (impl_) {
        impl_->loopType = loopType;
    }
}

ofLoopType ofVideoPlayer::getLoopState() const {
    return impl_ ? impl_->loopType : OF_LOOP_NONE;
}

// ============================================================================
// Position & Seeking
// ============================================================================

float ofVideoPlayer::getPosition() const {
    if (!impl_ || !impl_->loaded || impl_->duration <= 0) return 0.0f;
    return getCurrentTime() / impl_->duration;
}

void ofVideoPlayer::setPosition(float pct) {
    if (!impl_ || !impl_->loaded) return;

    pct = std::max(0.0f, std::min(1.0f, pct));
    setTime(pct * impl_->duration);
}

float ofVideoPlayer::getCurrentTime() const {
    if (!impl_ || !impl_->loaded) return 0.0f;

    @autoreleasepool {
        CMTime currentTime = [impl_->player currentTime];
        return static_cast<float>(CMTimeGetSeconds(currentTime));
    }
}

void ofVideoPlayer::setTime(float seconds) {
    if (!impl_ || !impl_->loaded) return;

    @autoreleasepool {
        seconds = std::max(0.0f, std::min(seconds, impl_->duration));
        CMTime time = CMTimeMakeWithSeconds(seconds, NSEC_PER_SEC);
        [impl_->player seekToTime:time toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
    }
}

int ofVideoPlayer::getCurrentFrame() const {
    if (!impl_ || !impl_->loaded || impl_->frameRate <= 0) return 0;
    return static_cast<int>(getCurrentTime() * impl_->frameRate);
}

void ofVideoPlayer::setFrame(int frame) {
    if (!impl_ || !impl_->loaded || impl_->frameRate <= 0) return;

    frame = std::max(0, std::min(frame, impl_->totalFrames - 1));
    setTime(static_cast<float>(frame) / impl_->frameRate);
}

void ofVideoPlayer::seekForward(float seconds) {
    setTime(getCurrentTime() + seconds);
}

void ofVideoPlayer::seekBackward(float seconds) {
    setTime(getCurrentTime() - seconds);
}

void ofVideoPlayer::firstFrame() {
    setFrame(0);
}

void ofVideoPlayer::nextFrame() {
    setFrame(getCurrentFrame() + 1);
}

void ofVideoPlayer::previousFrame() {
    setFrame(getCurrentFrame() - 1);
}

// ============================================================================
// Speed Control
// ============================================================================

float ofVideoPlayer::getSpeed() const {
    return impl_ ? impl_->speed : 1.0f;
}

void ofVideoPlayer::setSpeed(float speed) {
    if (!impl_) return;

    impl_->speed = std::max(0.0f, speed);

    if (impl_->loaded && impl_->playing) {
        @autoreleasepool {
            impl_->player.rate = impl_->playingForward ? impl_->speed : -impl_->speed;
        }
    }
}

// ============================================================================
// Audio Control
// ============================================================================

float ofVideoPlayer::getVolume() const {
    return impl_ ? impl_->volume : 1.0f;
}

void ofVideoPlayer::setVolume(float volume) {
    if (!impl_) return;

    impl_->volume = std::max(0.0f, std::min(1.0f, volume));

    if (impl_->loaded) {
        @autoreleasepool {
            impl_->player.volume = impl_->volume;
        }
    }
}

// ============================================================================
// Frame Update
// ============================================================================

void ofVideoPlayer::update() {
    if (!impl_ || !impl_->loaded) return;

    impl_->frameNew = false;

    @autoreleasepool {
        CMTime currentTime = [impl_->player currentTime];

        if ([impl_->videoOutput hasNewPixelBufferForItemTime:currentTime]) {
            CVPixelBufferRef pixelBuffer = [impl_->videoOutput copyPixelBufferForItemTime:currentTime
                                                                       itemTimeForDisplay:nil];
            if (pixelBuffer) {
                // Lock the pixel buffer
                CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

                // Get pixel data
                size_t width = CVPixelBufferGetWidth(pixelBuffer);
                size_t height = CVPixelBufferGetHeight(pixelBuffer);
                size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);
                void* baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);

                // Update pixels (BGRA format)
                if (impl_->pixels.getWidth() != width || impl_->pixels.getHeight() != height) {
                    impl_->pixels.allocate(width, height, 4);
                }

                // Convert BGRA to RGBA
                unsigned char* srcData = static_cast<unsigned char*>(baseAddress);
                unsigned char* dstData = impl_->pixels.getData();

                for (size_t y = 0; y < height; ++y) {
                    unsigned char* srcRow = srcData + y * bytesPerRow;
                    unsigned char* dstRow = dstData + y * width * 4;

                    for (size_t x = 0; x < width; ++x) {
                        dstRow[x * 4 + 0] = srcRow[x * 4 + 2]; // R from B
                        dstRow[x * 4 + 1] = srcRow[x * 4 + 1]; // G
                        dstRow[x * 4 + 2] = srcRow[x * 4 + 0]; // B from R
                        dstRow[x * 4 + 3] = srcRow[x * 4 + 3]; // A
                    }
                }

                // Unlock pixel buffer
                CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
                CVPixelBufferRelease(pixelBuffer);

                // Update texture
                impl_->texture.loadData(impl_->pixels);

                impl_->frameNew = true;
            }
        }
    }
}

bool ofVideoPlayer::isFrameNew() const {
    return impl_ && impl_->frameNew;
}

// ============================================================================
// Drawing
// ============================================================================

void ofVideoPlayer::draw(float x, float y) const {
    if (!impl_ || !impl_->loaded) return;
    impl_->texture.draw(x, y);
}

void ofVideoPlayer::draw(float x, float y, float w, float h) const {
    if (!impl_ || !impl_->loaded) return;
    impl_->texture.draw(x, y, w, h);
}

// ============================================================================
// Texture & Pixel Access
// ============================================================================

ofTexture& ofVideoPlayer::getTexture() {
    ensureImpl();
    return impl_->texture;
}

const ofTexture& ofVideoPlayer::getTexture() const {
    return impl_->texture;
}

ofPixels& ofVideoPlayer::getPixels() {
    ensureImpl();
    return impl_->pixels;
}

const ofPixels& ofVideoPlayer::getPixels() const {
    return impl_->pixels;
}

// ============================================================================
// Video Properties
// ============================================================================

float ofVideoPlayer::getWidth() const {
    return impl_ ? static_cast<float>(impl_->width) : 0.0f;
}

float ofVideoPlayer::getHeight() const {
    return impl_ ? static_cast<float>(impl_->height) : 0.0f;
}

float ofVideoPlayer::getDuration() const {
    return impl_ ? impl_->duration : 0.0f;
}

int ofVideoPlayer::getTotalNumFrames() const {
    return impl_ ? impl_->totalFrames : 0;
}

float ofVideoPlayer::getFrameRate() const {
    return impl_ ? impl_->frameRate : 0.0f;
}

bool ofVideoPlayer::hasAudio() const {
    return impl_ && impl_->hasAudioTrack;
}

} // namespace oflike

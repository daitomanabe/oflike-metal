#import "ofVideoGrabber.h"
#import <AVFoundation/AVFoundation.h>
#import <CoreVideo/CoreVideo.h>
#import "../../core/Context.h"

// ============================================================================
// AVCaptureVideoDataOutputSampleBufferDelegate Implementation
// ============================================================================

@interface OflikeCaptureDelegate : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

@property (nonatomic, assign) oflike::ofPixels* pixels;
@property (nonatomic, assign) oflike::ofTexture* texture;
@property (nonatomic, assign) bool frameNew;
@property (nonatomic, assign) int width;
@property (nonatomic, assign) int height;
@property (nonatomic, strong) dispatch_queue_t processingQueue;

@end

@implementation OflikeCaptureDelegate

- (instancetype)init {
    self = [super init];
    if (self) {
        _pixels = nullptr;
        _texture = nullptr;
        _frameNew = false;
        _width = 0;
        _height = 0;
        _processingQueue = dispatch_queue_create("com.oflike.capture", DISPATCH_QUEUE_SERIAL);
    }
    return self;
}

- (void)captureOutput:(AVCaptureOutput*)captureOutput
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection*)connection {

    (void)captureOutput;
    (void)connection;

    if (!_pixels || !_texture) {
        return;
    }

    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    if (!imageBuffer) {
        return;
    }

    CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);

    size_t width = CVPixelBufferGetWidth(imageBuffer);
    size_t height = CVPixelBufferGetHeight(imageBuffer);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer);
    void* baseAddress = CVPixelBufferGetBaseAddress(imageBuffer);

    // Update dimensions if changed
    if (_pixels->getWidth() != width || _pixels->getHeight() != height) {
        _pixels->allocate(width, height, 4);
        _width = static_cast<int>(width);
        _height = static_cast<int>(height);
    }

    // Convert BGRA to RGBA
    unsigned char* srcData = static_cast<unsigned char*>(baseAddress);
    unsigned char* dstData = _pixels->getData();

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

    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);

    _frameNew = true;
}

@end

namespace oflike {

// ============================================================================
// ofVideoGrabber::Impl
// ============================================================================

struct ofVideoGrabber::Impl {
    AVCaptureSession* captureSession = nil;
    AVCaptureDevice* captureDevice = nil;
    AVCaptureDeviceInput* captureInput = nil;
    AVCaptureVideoDataOutput* captureOutput = nil;
    OflikeCaptureDelegate* captureDelegate = nil;

    ofTexture texture;
    ofPixels pixels;

    int width = 0;
    int height = 0;
    int requestedWidth = 640;
    int requestedHeight = 480;
    float desiredFrameRate = 30.0f;
    float actualFrameRate = 30.0f;

    int deviceID = 0;
    std::string deviceUniqueID;

    bool initialized = false;
    bool frameNew = false;
    bool textureNeedsUpdate = false;

    Impl() = default;

    ~Impl() {
        close();
    }

    void close() {
        @autoreleasepool {
            if (captureSession) {
                [captureSession stopRunning];

                if (captureInput) {
                    [captureSession removeInput:captureInput];
                }
                if (captureOutput) {
                    [captureSession removeOutput:captureOutput];
                }

                captureSession = nil;
            }

            captureDevice = nil;
            captureInput = nil;
            captureOutput = nil;
            captureDelegate = nil;

            texture.clear();
            pixels.clear();

            width = 0;
            height = 0;
            initialized = false;
            frameNew = false;
            textureNeedsUpdate = false;
        }
    }
};

// ============================================================================
// ofVideoGrabber Implementation
// ============================================================================

ofVideoGrabber::ofVideoGrabber()
    : impl_(std::make_unique<Impl>()) {
}

ofVideoGrabber::~ofVideoGrabber() = default;

ofVideoGrabber::ofVideoGrabber(ofVideoGrabber&& other) noexcept = default;

ofVideoGrabber& ofVideoGrabber::operator=(ofVideoGrabber&& other) noexcept = default;

void ofVideoGrabber::ensureImpl() {
    if (!impl_) {
        impl_ = std::make_unique<Impl>();
    }
}

// ============================================================================
// Device Selection
// ============================================================================

std::vector<ofVideoDevice> ofVideoGrabber::listDevices() {
    std::vector<ofVideoDevice> devices;

    @autoreleasepool {
        AVCaptureDeviceDiscoverySession* discoverySession = [AVCaptureDeviceDiscoverySession
            discoverySessionWithDeviceTypes:@[
                AVCaptureDeviceTypeBuiltInWideAngleCamera,
                AVCaptureDeviceTypeExternalUnknown
            ]
            mediaType:AVMediaTypeVideo
            position:AVCaptureDevicePositionUnspecified];

        NSArray<AVCaptureDevice*>* captureDevices = [discoverySession devices];

        AVCaptureDevice* defaultDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];

        int index = 0;
        for (AVCaptureDevice* device in captureDevices) {
            ofVideoDevice vd;
            vd.id = index;
            vd.name = [[device localizedName] UTF8String];
            vd.uniqueID = [[device uniqueID] UTF8String];
            vd.isDefault = (defaultDevice && [[device uniqueID] isEqualToString:[defaultDevice uniqueID]]);

            devices.push_back(vd);
            ++index;

            NSLog(@"ofVideoGrabber: Found device [%d] %s%s",
                  vd.id, vd.name.c_str(), vd.isDefault ? " (default)" : "");
        }
    }

    return devices;
}

void ofVideoGrabber::setDeviceID(int deviceID) {
    ensureImpl();
    impl_->deviceID = deviceID;
    impl_->deviceUniqueID.clear();
}

void ofVideoGrabber::setDeviceID(const std::string& uniqueID) {
    ensureImpl();
    impl_->deviceUniqueID = uniqueID;
    impl_->deviceID = -1;
}

int ofVideoGrabber::getDeviceID() const {
    return impl_ ? impl_->deviceID : 0;
}

// ============================================================================
// Setup & Configuration
// ============================================================================

bool ofVideoGrabber::setup(int w, int h) {
    ensureImpl();

    // Close any existing capture
    close();

    impl_->requestedWidth = w;
    impl_->requestedHeight = h;

    @autoreleasepool {
        // Find the requested device
        AVCaptureDevice* device = nil;

        if (!impl_->deviceUniqueID.empty()) {
            // Find by unique ID
            NSString* uniqueID = [NSString stringWithUTF8String:impl_->deviceUniqueID.c_str()];
            device = [AVCaptureDevice deviceWithUniqueID:uniqueID];
        } else {
            // Find by index
            AVCaptureDeviceDiscoverySession* discoverySession = [AVCaptureDeviceDiscoverySession
                discoverySessionWithDeviceTypes:@[
                    AVCaptureDeviceTypeBuiltInWideAngleCamera,
                    AVCaptureDeviceTypeExternalUnknown
                ]
                mediaType:AVMediaTypeVideo
                position:AVCaptureDevicePositionUnspecified];

            NSArray<AVCaptureDevice*>* devices = [discoverySession devices];

            if (impl_->deviceID >= 0 && impl_->deviceID < static_cast<int>([devices count])) {
                device = devices[impl_->deviceID];
            } else if ([devices count] > 0) {
                device = devices[0];
            }
        }

        if (!device) {
            // Fall back to default device
            device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
        }

        if (!device) {
            NSLog(@"ofVideoGrabber: No camera device available");
            return false;
        }

        impl_->captureDevice = device;

        // Configure device for best matching format
        NSError* error = nil;
        if ([device lockForConfiguration:&error]) {
            // Find best matching format
            AVCaptureDeviceFormat* bestFormat = nil;
            Float64 bestScore = INFINITY;

            for (AVCaptureDeviceFormat* format in [device formats]) {
                CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions(format.formatDescription);

                // Calculate score (prefer closest to requested size)
                Float64 widthDiff = std::abs(dimensions.width - w);
                Float64 heightDiff = std::abs(dimensions.height - h);
                Float64 score = widthDiff + heightDiff;

                // Check if format supports desired frame rate
                for (AVFrameRateRange* range in format.videoSupportedFrameRateRanges) {
                    if (range.minFrameRate <= impl_->desiredFrameRate &&
                        range.maxFrameRate >= impl_->desiredFrameRate) {
                        // Bonus for supporting desired frame rate
                        score -= 1000;
                    }
                }

                if (score < bestScore) {
                    bestScore = score;
                    bestFormat = format;
                }
            }

            if (bestFormat) {
                device.activeFormat = bestFormat;

                // Set frame rate
                CMTime frameDuration = CMTimeMake(1, static_cast<int32_t>(impl_->desiredFrameRate));
                for (AVFrameRateRange* range in bestFormat.videoSupportedFrameRateRanges) {
                    if (range.minFrameRate <= impl_->desiredFrameRate &&
                        range.maxFrameRate >= impl_->desiredFrameRate) {
                        device.activeVideoMinFrameDuration = frameDuration;
                        device.activeVideoMaxFrameDuration = frameDuration;
                        break;
                    }
                }

                CMVideoDimensions dims = CMVideoFormatDescriptionGetDimensions(bestFormat.formatDescription);
                impl_->width = dims.width;
                impl_->height = dims.height;
            }

            [device unlockForConfiguration];
        } else {
            NSLog(@"ofVideoGrabber: Failed to lock device for configuration: %@", error);
        }

        // Create capture session
        impl_->captureSession = [[AVCaptureSession alloc] init];
        if (!impl_->captureSession) {
            NSLog(@"ofVideoGrabber: Failed to create capture session");
            return false;
        }

        // Create input
        impl_->captureInput = [AVCaptureDeviceInput deviceInputWithDevice:device error:&error];
        if (!impl_->captureInput) {
            NSLog(@"ofVideoGrabber: Failed to create capture input: %@", error);
            impl_->captureSession = nil;
            return false;
        }

        if (![impl_->captureSession canAddInput:impl_->captureInput]) {
            NSLog(@"ofVideoGrabber: Cannot add capture input to session");
            impl_->captureInput = nil;
            impl_->captureSession = nil;
            return false;
        }
        [impl_->captureSession addInput:impl_->captureInput];

        // Create output
        impl_->captureOutput = [[AVCaptureVideoDataOutput alloc] init];
        if (!impl_->captureOutput) {
            NSLog(@"ofVideoGrabber: Failed to create capture output");
            [impl_->captureSession removeInput:impl_->captureInput];
            impl_->captureInput = nil;
            impl_->captureSession = nil;
            return false;
        }

        // Configure output for BGRA format (Metal compatible)
        impl_->captureOutput.videoSettings = @{
            (id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)
        };
        impl_->captureOutput.alwaysDiscardsLateVideoFrames = YES;

        // Create delegate
        impl_->captureDelegate = [[OflikeCaptureDelegate alloc] init];
        impl_->captureDelegate.pixels = &impl_->pixels;
        impl_->captureDelegate.texture = &impl_->texture;

        dispatch_queue_t captureQueue = dispatch_queue_create("com.oflike.captureQueue", DISPATCH_QUEUE_SERIAL);
        [impl_->captureOutput setSampleBufferDelegate:impl_->captureDelegate queue:captureQueue];

        if (![impl_->captureSession canAddOutput:impl_->captureOutput]) {
            NSLog(@"ofVideoGrabber: Cannot add capture output to session");
            [impl_->captureSession removeInput:impl_->captureInput];
            impl_->captureInput = nil;
            impl_->captureOutput = nil;
            impl_->captureSession = nil;
            return false;
        }
        [impl_->captureSession addOutput:impl_->captureOutput];

        // Allocate texture and pixels
        if (impl_->width <= 0) impl_->width = w;
        if (impl_->height <= 0) impl_->height = h;

        impl_->texture.allocate(impl_->width, impl_->height, OF_IMAGE_COLOR_ALPHA);
        impl_->pixels.allocate(impl_->width, impl_->height, 4);

        // Start capture
        [impl_->captureSession startRunning];

        impl_->initialized = true;

        NSLog(@"ofVideoGrabber: Started capture %dx%d from %@",
              impl_->width, impl_->height, [device localizedName]);

        return true;
    }
}

void ofVideoGrabber::close() {
    if (impl_) {
        impl_->close();
    }
}

bool ofVideoGrabber::isInitialized() const {
    return impl_ && impl_->initialized;
}

void ofVideoGrabber::setDesiredFrameRate(float framerate) {
    if (impl_) {
        impl_->desiredFrameRate = std::max(1.0f, framerate);
    }
}

float ofVideoGrabber::getFrameRate() const {
    return impl_ ? impl_->actualFrameRate : 0.0f;
}

// ============================================================================
// Frame Update
// ============================================================================

void ofVideoGrabber::update() {
    if (!impl_ || !impl_->initialized) return;

    impl_->frameNew = false;

    @autoreleasepool {
        if (impl_->captureDelegate.frameNew) {
            // Update texture from pixels on main thread
            impl_->texture.loadData(impl_->pixels);
            impl_->frameNew = true;
            impl_->captureDelegate.frameNew = false;

            // Update actual dimensions
            impl_->width = impl_->captureDelegate.width;
            impl_->height = impl_->captureDelegate.height;
        }
    }
}

bool ofVideoGrabber::isFrameNew() const {
    return impl_ && impl_->frameNew;
}

// ============================================================================
// Drawing
// ============================================================================

void ofVideoGrabber::draw(float x, float y) const {
    if (!impl_ || !impl_->initialized) return;
    impl_->texture.draw(x, y);
}

void ofVideoGrabber::draw(float x, float y, float w, float h) const {
    if (!impl_ || !impl_->initialized) return;
    impl_->texture.draw(x, y, w, h);
}

// ============================================================================
// Texture & Pixel Access
// ============================================================================

ofTexture& ofVideoGrabber::getTexture() {
    ensureImpl();
    return impl_->texture;
}

const ofTexture& ofVideoGrabber::getTexture() const {
    return impl_->texture;
}

ofPixels& ofVideoGrabber::getPixels() {
    ensureImpl();
    return impl_->pixels;
}

const ofPixels& ofVideoGrabber::getPixels() const {
    return impl_->pixels;
}

// ============================================================================
// Camera Properties
// ============================================================================

float ofVideoGrabber::getWidth() const {
    return impl_ ? static_cast<float>(impl_->width) : 0.0f;
}

float ofVideoGrabber::getHeight() const {
    return impl_ ? static_cast<float>(impl_->height) : 0.0f;
}

// ============================================================================
// Permissions
// ============================================================================

bool ofVideoGrabber::isAuthorized() {
    @autoreleasepool {
        AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
        return status == AVAuthorizationStatusAuthorized;
    }
}

void ofVideoGrabber::requestPermission() {
    @autoreleasepool {
        [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted) {
            if (granted) {
                NSLog(@"ofVideoGrabber: Camera access granted");
            } else {
                NSLog(@"ofVideoGrabber: Camera access denied");
            }
        }];
    }
}

} // namespace oflike

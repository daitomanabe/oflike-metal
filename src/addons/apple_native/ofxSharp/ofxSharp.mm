#import "ofxSharp.h"
#import "SharpModel.h"
#import "SharpGaussianCloud.h"
#import "SharpRenderer.h"
#import "ofPixels.h"
#import "ofTexture.h"
#import "ofCamera.h"
#import "ofImage.h"
#import "ofLog.h"
#import "math/ofMatrix4x4.h"
#import <Metal/Metal.h>
#import <Foundation/Foundation.h>
#import <dispatch/dispatch.h>

using namespace oflike;

// ============================================================================
// ofxSharp::Impl - Implementation class
// ============================================================================

class ofxSharp::Impl {
public:
    // Sharp components
    std::unique_ptr<Sharp::SharpModel> model;
    std::unique_ptr<Sharp::SharpRenderer> renderer;
    std::unique_ptr<Sharp::GaussianCloud> cloud;

    // State
    bool isSetup = false;
    bool isGenerating = false;
    std::string lastError;

    // Configuration
    int renderWidth = 1920;
    int renderHeight = 1080;
    std::string modelPath = "sharp.mlmodelc";

    // Metal resources (from context)
    id<MTLDevice> device = nil;
    id<MTLCommandQueue> commandQueue = nil;

    // Async generation
    dispatch_queue_t asyncQueue = nullptr;

    // Constructor
    Impl() {
        @autoreleasepool {
            // Get Metal device and command queue from oflike context
            device = MTLCreateSystemDefaultDevice();
            if (!device) {
                lastError = "Failed to create Metal device";
                ofLogError("ofxSharp") << lastError;
                return;
            }

            commandQueue = [device newCommandQueue];
            if (!commandQueue) {
                lastError = "Failed to create Metal command queue";
                ofLogError("ofxSharp") << lastError;
                return;
            }

            // Create async dispatch queue
            asyncQueue = dispatch_queue_create("com.oflike.ofxSharp.async", DISPATCH_QUEUE_SERIAL);

            ofLogVerbose("ofxSharp") << "Impl created with Metal device: "
                                     << [device.name UTF8String];
        }
    }

    // Destructor
    ~Impl() {
        @autoreleasepool {
            model.reset();
            renderer.reset();
            cloud.reset();

            if (asyncQueue) {
                // Don't release on modern ARC, just nullify
                asyncQueue = nullptr;
            }

            commandQueue = nil;
            device = nil;

            ofLogVerbose("ofxSharp") << "Impl destroyed";
        }
    }

    // Setup implementation
    bool setupInternal(const std::string& modelPathArg, int width, int height) {
        @autoreleasepool {
            if (isSetup) {
                ofLogWarning("ofxSharp") << "Already setup, re-initializing";
                cleanup();
            }

            renderWidth = width;
            renderHeight = height;
            modelPath = modelPathArg;

            // Create Sharp components
            model = std::make_unique<Sharp::SharpModel>();
            renderer = std::make_unique<Sharp::SharpRenderer>();
            cloud = std::make_unique<Sharp::GaussianCloud>();

            // Load Core ML model
            ofLogNotice("ofxSharp") << "Loading SHARP model: " << modelPath;

            // Try to load from bundle first, then from filesystem
            NSString* modelName = [NSString stringWithUTF8String:modelPath.c_str()];
            NSString* modelPathNS = nil;

            // Check if it's a full path
            if ([modelName hasPrefix:@"/"]) {
                modelPathNS = modelName;
            } else {
                // Look in bundle
                NSString* nameWithoutExt = [modelName stringByDeletingPathExtension];
                NSString* ext = [modelName pathExtension];
                if ([ext length] == 0) {
                    ext = @"mlmodelc";
                }

                modelPathNS = [[NSBundle mainBundle] pathForResource:nameWithoutExt ofType:ext];

                // If not found in bundle, try relative to data folder
                if (!modelPathNS) {
                    // Try addon directory
                    NSString* addonPath = [NSString stringWithFormat:@"%s/src/addons/apple_native/ofxSharp/models/%@",
                                          ofFilePath::getCurrentExeDir().c_str(), modelName];
                    if ([[NSFileManager defaultManager] fileExistsAtPath:addonPath]) {
                        modelPathNS = addonPath;
                    }
                }
            }

            std::string finalModelPath;
            if (modelPathNS) {
                finalModelPath = [modelPathNS UTF8String];
            } else {
                finalModelPath = modelPath;
            }

            if (!model->load(finalModelPath)) {
                lastError = "Failed to load SHARP model: " + finalModelPath;
                ofLogError("ofxSharp") << lastError;
                ofLogError("ofxSharp") << "Model error: " << model->getLastError();
                return false;
            }

            ofLogNotice("ofxSharp") << "Model loaded successfully";

            auto modelInfo = model->getModelInfo();
            ofLogNotice("ofxSharp") << "  Input size: " << modelInfo.inputWidth << "x" << modelInfo.inputHeight;
            ofLogNotice("ofxSharp") << "  Max Gaussians: " << modelInfo.maxOutputGaussians;
            ofLogNotice("ofxSharp") << "  Neural Engine: " << (modelInfo.isNeuralEngineSupported ? "YES" : "NO");

            // Initialize renderer
            ofLogNotice("ofxSharp") << "Initializing renderer: " << width << "x" << height;

            if (!renderer->initialize((__bridge void*)device, (__bridge void*)commandQueue)) {
                lastError = "Failed to initialize renderer";
                ofLogError("ofxSharp") << lastError;
                return false;
            }

            ofLogNotice("ofxSharp") << "Renderer initialized";

            isSetup = true;
            lastError.clear();

            ofLogNotice("ofxSharp") << "ofxSharp setup complete";
            return true;
        }
    }

    // Cleanup
    void cleanup() {
        @autoreleasepool {
            model.reset();
            renderer.reset();
            cloud.reset();
            isSetup = false;
        }
    }

    // Generate from pixels (internal)
    bool generateFromPixelsInternal(const ofPixels& pixels) {
        @autoreleasepool {
            if (!isSetup || !model) {
                lastError = "ofxSharp not setup";
                ofLogError("ofxSharp") << lastError;
                return false;
            }

            ofLogNotice("ofxSharp") << "Generating Gaussian cloud from pixels ("
                                     << pixels.getWidth() << "x" << pixels.getHeight() << ")";

            // Run inference
            auto newCloud = model->predict(pixels);

            if (newCloud.empty()) {
                lastError = "Failed to generate Gaussian cloud: " + model->getLastError();
                ofLogError("ofxSharp") << lastError;
                return false;
            }

            // Replace current cloud
            *cloud = std::move(newCloud);

            ofLogNotice("ofxSharp") << "Generated " << cloud->size() << " Gaussians";
            ofLogNotice("ofxSharp") << "Inference time: " << model->getLastInferenceTime() << " ms";
            ofLogNotice("ofxSharp") << "Neural Engine: " << (model->wasNeuralEngineUsed() ? "YES" : "NO");

            // Update Metal buffer
            if (!cloud->updateMetalBuffer()) {
                lastError = "Failed to update Metal buffer";
                ofLogError("ofxSharp") << lastError;
                return false;
            }

            lastError.clear();
            return true;
        }
    }

    // Generate from texture (internal)
    bool generateFromTextureInternal(const ofTexture& texture) {
        @autoreleasepool {
            if (!isSetup || !model) {
                lastError = "ofxSharp not setup";
                ofLogError("ofxSharp") << lastError;
                return false;
            }

            ofLogNotice("ofxSharp") << "Generating Gaussian cloud from texture ("
                                     << texture.getWidth() << "x" << texture.getHeight() << ")";

            // Run inference
            auto newCloud = model->predict(texture);

            if (newCloud.empty()) {
                lastError = "Failed to generate Gaussian cloud: " + model->getLastError();
                ofLogError("ofxSharp") << lastError;
                return false;
            }

            // Replace current cloud
            *cloud = std::move(newCloud);

            ofLogNotice("ofxSharp") << "Generated " << cloud->size() << " Gaussians";
            ofLogNotice("ofxSharp") << "Inference time: " << model->getLastInferenceTime() << " ms";
            ofLogNotice("ofxSharp") << "Neural Engine: " << (model->wasNeuralEngineUsed() ? "YES" : "NO");

            // Update Metal buffer
            if (!cloud->updateMetalBuffer()) {
                lastError = "Failed to update Metal buffer";
                ofLogError("ofxSharp") << lastError;
                return false;
            }

            lastError.clear();
            return true;
        }
    }
};

// ============================================================================
// ofxSharp - Public API Implementation
// ============================================================================

ofxSharp::ofxSharp()
    : impl_(std::make_unique<Impl>())
{
}

ofxSharp::~ofxSharp() = default;

ofxSharp::ofxSharp(ofxSharp&& other) noexcept = default;

ofxSharp& ofxSharp::operator=(ofxSharp&& other) noexcept = default;

// ============================================================================
// Setup
// ============================================================================

bool ofxSharp::setup() {
    return setup("sharp.mlmodelc", 1920, 1080);
}

bool ofxSharp::setup(const std::string& modelPath) {
    return setup(modelPath, 1920, 1080);
}

bool ofxSharp::setup(int width, int height) {
    return setup("sharp.mlmodelc", width, height);
}

bool ofxSharp::setup(const std::string& modelPath, int width, int height) {
    return impl_->setupInternal(modelPath, width, height);
}

bool ofxSharp::isSetup() const {
    return impl_->isSetup;
}

// ============================================================================
// Generation (Synchronous)
// ============================================================================

bool ofxSharp::generateFromImage(const ofPixels& pixels) {
    return impl_->generateFromPixelsInternal(pixels);
}

bool ofxSharp::generateFromImage(const ofTexture& texture) {
    return impl_->generateFromTextureInternal(texture);
}

bool ofxSharp::generateFromImage(const ofImage& image) {
    return generateFromImage(image.getPixels());
}

bool ofxSharp::loadCloud(const std::string& filepath) {
    @autoreleasepool {
        if (!impl_->isSetup || !impl_->cloud) {
            impl_->lastError = "ofxSharp not setup";
            ofLogError("ofxSharp") << impl_->lastError;
            return false;
        }

        ofLogNotice("ofxSharp") << "Loading cloud from: " << filepath;

        if (!impl_->cloud->loadFromPLY(filepath)) {
            impl_->lastError = "Failed to load cloud from PLY file";
            ofLogError("ofxSharp") << impl_->lastError;
            return false;
        }

        ofLogNotice("ofxSharp") << "Loaded " << impl_->cloud->size() << " Gaussians";

        // Update Metal buffer
        if (!impl_->cloud->updateMetalBuffer()) {
            impl_->lastError = "Failed to update Metal buffer";
            ofLogError("ofxSharp") << impl_->lastError;
            return false;
        }

        impl_->lastError.clear();
        return true;
    }
}

bool ofxSharp::saveCloud(const std::string& filepath) const {
    @autoreleasepool {
        if (!impl_->isSetup || !impl_->cloud || impl_->cloud->empty()) {
            impl_->lastError = "No cloud to save";
            ofLogError("ofxSharp") << impl_->lastError;
            return false;
        }

        ofLogNotice("ofxSharp") << "Saving cloud to: " << filepath;

        if (!impl_->cloud->saveToPLY(filepath)) {
            impl_->lastError = "Failed to save cloud to PLY file";
            ofLogError("ofxSharp") << impl_->lastError;
            return false;
        }

        ofLogNotice("ofxSharp") << "Saved " << impl_->cloud->size() << " Gaussians";

        impl_->lastError.clear();
        return true;
    }
}

// ============================================================================
// Generation (Asynchronous)
// ============================================================================

void ofxSharp::generateFromImageAsync(const ofPixels& pixels, GenerateCallback callback) {
    @autoreleasepool {
        if (!impl_->isSetup || !impl_->model) {
            impl_->lastError = "ofxSharp not setup";
            ofLogError("ofxSharp") << impl_->lastError;
            if (callback) {
                callback(false);
            }
            return;
        }

        if (impl_->isGenerating) {
            impl_->lastError = "Generation already in progress";
            ofLogWarning("ofxSharp") << impl_->lastError;
            if (callback) {
                callback(false);
            }
            return;
        }

        impl_->isGenerating = true;

        // Copy pixels for async processing
        ofPixels pixelsCopy = pixels;

        ofLogNotice("ofxSharp") << "Starting async generation";

        // Launch async inference
        impl_->model->predictAsync(pixelsCopy, [this, callback](Sharp::GaussianCloud&& newCloud, Sharp::ModelStatus status) {
            @autoreleasepool {
                bool success = (status == Sharp::ModelStatus::Success) && !newCloud.empty();

                if (success) {
                    // Replace cloud (this should be thread-safe as Sharp uses move semantics)
                    *impl_->cloud = std::move(newCloud);
                    impl_->cloud->updateMetalBuffer();

                    ofLogNotice("ofxSharp") << "Async generation complete: " << impl_->cloud->size() << " Gaussians";
                } else {
                    impl_->lastError = "Async generation failed: " + impl_->model->getLastError();
                    ofLogError("ofxSharp") << impl_->lastError;
                }

                impl_->isGenerating = false;

                // Call user callback on main thread
                if (callback) {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        callback(success);
                    });
                }
            }
        });
    }
}

void ofxSharp::generateFromImageAsync(const ofTexture& texture, GenerateCallback callback) {
    @autoreleasepool {
        if (!impl_->isSetup || !impl_->model) {
            impl_->lastError = "ofxSharp not setup";
            ofLogError("ofxSharp") << impl_->lastError;
            if (callback) {
                callback(false);
            }
            return;
        }

        if (impl_->isGenerating) {
            impl_->lastError = "Generation already in progress";
            ofLogWarning("ofxSharp") << impl_->lastError;
            if (callback) {
                callback(false);
            }
            return;
        }

        impl_->isGenerating = true;

        ofLogNotice("ofxSharp") << "Starting async generation from texture";

        // Launch async inference
        impl_->model->predictAsync(texture, [this, callback](Sharp::GaussianCloud&& newCloud, Sharp::ModelStatus status) {
            @autoreleasepool {
                bool success = (status == Sharp::ModelStatus::Success) && !newCloud.empty();

                if (success) {
                    *impl_->cloud = std::move(newCloud);
                    impl_->cloud->updateMetalBuffer();

                    ofLogNotice("ofxSharp") << "Async generation complete: " << impl_->cloud->size() << " Gaussians";
                } else {
                    impl_->lastError = "Async generation failed: " + impl_->model->getLastError();
                    ofLogError("ofxSharp") << impl_->lastError;
                }

                impl_->isGenerating = false;

                if (callback) {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        callback(success);
                    });
                }
            }
        });
    }
}

void ofxSharp::generateFromImageAsync(const ofImage& image, GenerateCallback callback) {
    generateFromImageAsync(image.getPixels(), callback);
}

bool ofxSharp::isGenerating() const {
    return impl_->isGenerating;
}

void ofxSharp::cancelGeneration() {
    if (impl_->isGenerating && impl_->model) {
        impl_->model->cancelInference();
        impl_->isGenerating = false;
        ofLogNotice("ofxSharp") << "Generation cancelled";
    }
}

// ============================================================================
// Rendering
// ============================================================================

void ofxSharp::drawCloud(const ofCamera& camera) {
    @autoreleasepool {
        if (!impl_->isSetup || !impl_->renderer || !impl_->cloud || impl_->cloud->empty()) {
            return;
        }

        // Get current Metal render target and command buffer from oflike context
        // For now, we'll use the renderer's internal render method
        // This will be integrated with oflike's render pipeline

        // Note: This is a simplified implementation
        // In a full integration, we would get the current MTLRenderCommandEncoder
        // from the oflike rendering context and render directly to it

        ofLogWarning("ofxSharp") << "drawCloud() requires integration with oflike render pipeline";
        ofLogWarning("ofxSharp") << "Use getRenderer() for manual rendering";
    }
}

void ofxSharp::drawCloud() {
    @autoreleasepool {
        // Get current camera from oflike context
        // This requires integration with oflike's camera stack
        ofLogWarning("ofxSharp") << "drawCloud() without camera requires integration with oflike camera stack";
        ofLogWarning("ofxSharp") << "Use drawCloud(camera) instead";
    }
}

bool ofxSharp::hasCloud() const {
    return impl_->isSetup && impl_->cloud && !impl_->cloud->empty();
}

size_t ofxSharp::getGaussianCount() const {
    if (impl_->cloud) {
        return impl_->cloud->size();
    }
    return 0;
}

// ============================================================================
// Cloud Manipulation
// ============================================================================

void ofxSharp::translate(float x, float y, float z) {
    if (impl_->cloud && !impl_->cloud->empty()) {
        impl_->cloud->translate(oflike::float3{x, y, z});
        impl_->cloud->updateMetalBuffer();
    }
}

void ofxSharp::rotate(float angle, float ax, float ay, float az) {
    if (impl_->cloud && !impl_->cloud->empty()) {
        oflike::float3 axis = simd_normalize(oflike::float3{ax, ay, az});
        oflike::quatf q = simd_quaternion(angle, axis);
        impl_->cloud->rotate(q);
        impl_->cloud->updateMetalBuffer();
    }
}

void ofxSharp::scale(float scale) {
    if (impl_->cloud && !impl_->cloud->empty()) {
        impl_->cloud->scale(scale);
        impl_->cloud->updateMetalBuffer();
    }
}

void ofxSharp::scale(float sx, float sy, float sz) {
    if (impl_->cloud && !impl_->cloud->empty()) {
        impl_->cloud->scale(oflike::float3{sx, sy, sz});
        impl_->cloud->updateMetalBuffer();
    }
}

void ofxSharp::resetTransform() {
    // Note: This would require storing the original cloud state
    // For now, this is a no-op
    ofLogWarning("ofxSharp") << "resetTransform() not yet implemented";
}

// ============================================================================
// Rendering Configuration
// ============================================================================

void ofxSharp::setDepthSortEnabled(bool enabled) {
    if (impl_->renderer) {
        impl_->renderer->setDepthSortEnabled(enabled);
    }
}

void ofxSharp::setSplatScale(float scale) {
    if (impl_->renderer) {
        impl_->renderer->setSplatScale(scale);
    }
}

void ofxSharp::setOpacityScale(float scale) {
    if (impl_->renderer) {
        impl_->renderer->setOpacityScale(scale);
    }
}

void ofxSharp::setAntiAliasingEnabled(bool enabled) {
    if (impl_->renderer) {
        auto config = impl_->renderer->getConfig();
        config.enableAntialiasing = enabled;
        impl_->renderer->setConfig(config);
    }
}

void ofxSharp::setSphericalHarmonicsEnabled(bool enabled) {
    if (impl_->renderer) {
        impl_->renderer->setSphericalHarmonicsEnabled(enabled);
    }
}

void ofxSharp::setMaxSHDegree(int degree) {
    if (impl_->renderer) {
        impl_->renderer->setMaxSHDegree(degree);
    }
}

const Sharp::RenderConfig& ofxSharp::getRenderConfig() const {
    static Sharp::RenderConfig defaultConfig;
    if (impl_->renderer) {
        return impl_->renderer->getConfig();
    }
    return defaultConfig;
}

const Sharp::RenderStats& ofxSharp::getRenderStats() const {
    static Sharp::RenderStats defaultStats;
    if (impl_->renderer) {
        return impl_->renderer->getStats();
    }
    return defaultStats;
}

// ============================================================================
// Filtering
// ============================================================================

void ofxSharp::filterByOpacity(float minOpacity) {
    if (impl_->cloud && !impl_->cloud->empty()) {
        impl_->cloud->filterByOpacity(minOpacity);
        impl_->cloud->updateMetalBuffer();
        ofLogNotice("ofxSharp") << "Filtered by opacity, " << impl_->cloud->size() << " Gaussians remaining";
    }
}

void ofxSharp::filterBySize(float minSize, float maxSize) {
    if (impl_->cloud && !impl_->cloud->empty()) {
        impl_->cloud->filterBySize(minSize, maxSize);
        impl_->cloud->updateMetalBuffer();
        ofLogNotice("ofxSharp") << "Filtered by size, " << impl_->cloud->size() << " Gaussians remaining";
    }
}

void ofxSharp::removeInvisible() {
    if (impl_->cloud && !impl_->cloud->empty()) {
        impl_->cloud->removeInvisible();
        impl_->cloud->updateMetalBuffer();
        ofLogNotice("ofxSharp") << "Removed invisible, " << impl_->cloud->size() << " Gaussians remaining";
    }
}

// ============================================================================
// Error Handling
// ============================================================================

std::string ofxSharp::getLastError() const {
    return impl_->lastError;
}

bool ofxSharp::isUsingNeuralEngine() const {
    if (impl_->model) {
        return impl_->model->wasNeuralEngineUsed();
    }
    return false;
}

double ofxSharp::getLastInferenceTime() const {
    if (impl_->model) {
        return impl_->model->getLastInferenceTime();
    }
    return 0.0;
}

// ============================================================================
// Advanced: Direct Access
// ============================================================================

Sharp::SharpModel* ofxSharp::getModel() {
    return impl_->model.get();
}

Sharp::SharpRenderer* ofxSharp::getRenderer() {
    return impl_->renderer.get();
}

Sharp::GaussianCloud* ofxSharp::getCloud() {
    return impl_->cloud.get();
}

const Sharp::GaussianCloud* ofxSharp::getCloud() const {
    return impl_->cloud.get();
}

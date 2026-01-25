#include "GenericModel.h"
#include "ofPixels.h"
#include "ofTexture.h"
#include <Foundation/Foundation.h>
#include <CoreML/CoreML.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <Metal/Metal.h>

using namespace oflike;

namespace NeuralEngine {

// ============================================================================
// Helper functions
// ============================================================================

static FeatureType convertMLFeatureType(MLFeatureType type) {
    switch (type) {
        case MLFeatureTypeInt64: return FeatureType::Int64;
        case MLFeatureTypeDouble: return FeatureType::Double;
        case MLFeatureTypeString: return FeatureType::String;
        case MLFeatureTypeImage: return FeatureType::Image;
        case MLFeatureTypeMultiArray: return FeatureType::MultiArray;
        case MLFeatureTypeDictionary: return FeatureType::Dictionary;
        case MLFeatureTypeSequence: return FeatureType::Sequence;
        default: return FeatureType::Invalid;
    }
}

static MultiArrayDataType convertMLMultiArrayDataType(MLMultiArrayDataType type) {
    switch (type) {
        case MLMultiArrayDataTypeDouble: return MultiArrayDataType::Double;
        case MLMultiArrayDataTypeFloat32: return MultiArrayDataType::Float32;
        case MLMultiArrayDataTypeFloat16: return MultiArrayDataType::Float16;
        case MLMultiArrayDataTypeInt32: return MultiArrayDataType::Int32;
        default: return MultiArrayDataType::Invalid;
    }
}

static FeatureInfo extractFeatureInfo(MLFeatureDescription* desc) {
    @autoreleasepool {
        FeatureInfo info;
        info.name = [desc.name UTF8String];
        info.type = convertMLFeatureType(desc.type);
        info.isOptional = desc.optional;

        // Extract type-specific information
        if (desc.type == MLFeatureTypeMultiArray) {
            MLMultiArrayConstraint* constraint = desc.multiArrayConstraint;
            if (constraint) {
                info.arrayDataType = convertMLMultiArrayDataType(constraint.dataType);

                // Extract shape
                for (NSNumber* dim in constraint.shape) {
                    info.arrayShape.push_back([dim unsignedLongValue]);
                }
            }
        } else if (desc.type == MLFeatureTypeImage) {
            MLImageConstraint* constraint = desc.imageConstraint;
            if (constraint) {
                info.imageWidth = constraint.pixelsWide;
                info.imageHeight = constraint.pixelsHigh;
            }
        } else if (desc.type == MLFeatureTypeDictionary) {
            // Core ML dictionary constraints (if available)
            // Note: Dictionary constraints are limited in Core ML API
        }

        return info;
    }
}

// ============================================================================
// Implementation (pImpl pattern)
// ============================================================================

struct GenericModel::Impl {
    MLModel* model = nil;
    GenericModelConfig config;
    GenericModelInfo info;
    std::string lastError;

    // Input/output features
    NSMutableDictionary<NSString*, MLFeatureValue*>* inputFeatures = nil;
    MLFeatureProvider* outputFeatures = nil;

    Impl() {
        @autoreleasepool {
            inputFeatures = [[NSMutableDictionary alloc] init];
        }
    }

    ~Impl() {
        @autoreleasepool {
            model = nil;
            inputFeatures = nil;
            outputFeatures = nil;
        }
    }

    bool loadModel(const std::string& modelPath, const GenericModelConfig& cfg) {
        @autoreleasepool {
            config = cfg;

            // Convert path to NSURL
            NSString* path = [NSString stringWithUTF8String:modelPath.c_str()];
            NSURL* modelURL = [NSURL fileURLWithPath:path];

            // Create model configuration
            MLModelConfiguration* configuration = [[MLModelConfiguration alloc] init];

            // Set compute units
            switch (config.computeUnits) {
                case GenericModelConfig::ComputeUnits::CPUOnly:
                    configuration.computeUnits = MLComputeUnitsCPUOnly;
                    break;
                case GenericModelConfig::ComputeUnits::CPUAndGPU:
                    configuration.computeUnits = MLComputeUnitsCPUAndGPU;
                    break;
                case GenericModelConfig::ComputeUnits::CPUAndNeuralEngine:
                    configuration.computeUnits = MLComputeUnitsCPUAndNeuralEngine;
                    break;
                case GenericModelConfig::ComputeUnits::All:
                default:
                    configuration.computeUnits = MLComputeUnitsAll;
                    break;
            }

            // Load model
            NSError* error = nil;
            model = [MLModel modelWithContentsOfURL:modelURL configuration:configuration error:&error];

            if (error || !model) {
                lastError = error ? [[error localizedDescription] UTF8String] : "Failed to load model";
                return false;
            }

            // Extract model information
            info.modelPath = modelPath;
            info.isLoaded = true;

            MLModelDescription* description = model.modelDescription;

            // Extract metadata
            NSDictionary* metadata = description.metadata;
            if (metadata) {
                if (metadata[MLModelDescriptionKey]) {
                    info.modelDescription = [metadata[MLModelDescriptionKey] UTF8String];
                }
                if (metadata[MLModelAuthorKey]) {
                    info.modelAuthor = [metadata[MLModelAuthorKey] UTF8String];
                }
                if (metadata[MLModelLicenseKey]) {
                    info.modelLicense = [metadata[MLModelLicenseKey] UTF8String];
                }
                if (metadata[MLModelVersionStringKey]) {
                    info.modelVersion = [metadata[MLModelVersionStringKey] UTF8String];
                }
            }

            // Extract input descriptions
            info.inputs.clear();
            for (MLFeatureDescription* input in description.inputDescriptionsByName.allValues) {
                info.inputs.push_back(extractFeatureInfo(input));
            }

            // Extract output descriptions
            info.outputs.clear();
            for (MLFeatureDescription* output in description.outputDescriptionsByName.allValues) {
                info.outputs.push_back(extractFeatureInfo(output));
            }

            // Check if Neural Engine is supported
            if (@available(macOS 13.0, *)) {
                info.isNeuralEngineSupported = (configuration.computeUnits == MLComputeUnitsAll ||
                                                configuration.computeUnits == MLComputeUnitsCPUAndNeuralEngine);
            }

            return true;
        }
    }

    bool setImageInput(const std::string& name, const ofPixels& pixels) {
        @autoreleasepool {
            if (!model) {
                lastError = "Model not loaded";
                return false;
            }

            // Create CGImage from ofPixels
            size_t width = pixels.getWidth();
            size_t height = pixels.getHeight();
            size_t bytesPerRow = width * pixels.getBytesPerPixel();

            CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
            CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big;

            CGDataProviderRef provider = CGDataProviderCreateWithData(
                nullptr,
                pixels.getData(),
                height * bytesPerRow,
                nullptr
            );

            CGImageRef cgImage = CGImageCreate(
                width, height,
                8, pixels.getBitsPerPixel(),
                bytesPerRow,
                colorSpace,
                bitmapInfo,
                provider,
                nullptr, false,
                kCGRenderingIntentDefault
            );

            CGDataProviderRelease(provider);
            CGColorSpaceRelease(colorSpace);

            if (!cgImage) {
                lastError = "Failed to create CGImage";
                return false;
            }

            // Create MLFeatureValue from CGImage
            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];

            NSError* error = nil;
            MLFeatureValue* featureValue = [MLFeatureValue featureValueWithCGImage:cgImage error:&error];

            CGImageRelease(cgImage);

            if (error || !featureValue) {
                lastError = error ? [[error localizedDescription] UTF8String] : "Failed to create feature value";
                return false;
            }

            inputFeatures[featureName] = featureValue;
            return true;
        }
    }

    bool setInt64Input(const std::string& name, int64_t value) {
        @autoreleasepool {
            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            MLFeatureValue* featureValue = [MLFeatureValue featureValueWithInt64:value];
            inputFeatures[featureName] = featureValue;
            return true;
        }
    }

    bool setDoubleInput(const std::string& name, double value) {
        @autoreleasepool {
            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            MLFeatureValue* featureValue = [MLFeatureValue featureValueWithDouble:value];
            inputFeatures[featureName] = featureValue;
            return true;
        }
    }

    bool setStringInput(const std::string& name, const std::string& value) {
        @autoreleasepool {
            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            NSString* stringValue = [NSString stringWithUTF8String:value.c_str()];
            MLFeatureValue* featureValue = [MLFeatureValue featureValueWithString:stringValue];
            inputFeatures[featureName] = featureValue;
            return true;
        }
    }

    bool setMultiArrayInput(const std::string& name, const void* data,
                           const std::vector<size_t>& shape, MLMultiArrayDataType dataType) {
        @autoreleasepool {
            if (!model) {
                lastError = "Model not loaded";
                return false;
            }

            // Create NSArray from shape
            NSMutableArray<NSNumber*>* shapeArray = [NSMutableArray array];
            size_t totalElements = 1;
            for (size_t dim : shape) {
                [shapeArray addObject:@(dim)];
                totalElements *= dim;
            }

            // Create MLMultiArray
            NSError* error = nil;
            MLMultiArray* multiArray = [[MLMultiArray alloc] initWithShape:shapeArray
                                                                  dataType:dataType
                                                                     error:&error];

            if (error || !multiArray) {
                lastError = error ? [[error localizedDescription] UTF8String] : "Failed to create multi-array";
                return false;
            }

            // Copy data
            size_t elementSize = 0;
            switch (dataType) {
                case MLMultiArrayDataTypeFloat32:
                    elementSize = sizeof(float);
                    break;
                case MLMultiArrayDataTypeDouble:
                    elementSize = sizeof(double);
                    break;
                case MLMultiArrayDataTypeInt32:
                    elementSize = sizeof(int32_t);
                    break;
                case MLMultiArrayDataTypeFloat16:
                    elementSize = 2; // 16-bit float
                    break;
                default:
                    lastError = "Unsupported data type";
                    return false;
            }

            memcpy(multiArray.dataPointer, data, totalElements * elementSize);

            // Create feature value
            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            MLFeatureValue* featureValue = [MLFeatureValue featureValueWithMultiArray:multiArray];
            inputFeatures[featureName] = featureValue;

            return true;
        }
    }

    bool setDictionaryInput(const std::string& name, const std::map<std::string, double>& dict) {
        @autoreleasepool {
            NSMutableDictionary<NSString*, NSNumber*>* nsDict = [NSMutableDictionary dictionary];

            for (const auto& pair : dict) {
                NSString* key = [NSString stringWithUTF8String:pair.first.c_str()];
                nsDict[key] = @(pair.second);
            }

            NSError* error = nil;
            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            MLFeatureValue* featureValue = [MLFeatureValue featureValueWithDictionary:nsDict
                                                                                error:&error];

            if (error || !featureValue) {
                lastError = error ? [[error localizedDescription] UTF8String] : "Failed to create dictionary feature";
                return false;
            }

            inputFeatures[featureName] = featureValue;
            return true;
        }
    }

    bool runPrediction() {
        @autoreleasepool {
            if (!model) {
                lastError = "Model not loaded";
                return false;
            }

            // Create feature provider from input features
            MLDictionaryFeatureProvider* provider = [[MLDictionaryFeatureProvider alloc]
                                                     initWithDictionary:inputFeatures
                                                     error:nil];

            // Run prediction
            NSError* error = nil;
            outputFeatures = [model predictionFromFeatures:provider error:&error];

            if (error || !outputFeatures) {
                lastError = error ? [[error localizedDescription] UTF8String] : "Prediction failed";
                return false;
            }

            return true;
        }
    }

    int64_t getInt64Output(const std::string& name) const {
        @autoreleasepool {
            if (!outputFeatures) return 0;

            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            MLFeatureValue* value = [outputFeatures featureValueForName:featureName];

            if (value && value.type == MLFeatureTypeInt64) {
                return value.int64Value;
            }

            return 0;
        }
    }

    double getDoubleOutput(const std::string& name) const {
        @autoreleasepool {
            if (!outputFeatures) return 0.0;

            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            MLFeatureValue* value = [outputFeatures featureValueForName:featureName];

            if (value && value.type == MLFeatureTypeDouble) {
                return value.doubleValue;
            }

            return 0.0;
        }
    }

    std::string getStringOutput(const std::string& name) const {
        @autoreleasepool {
            if (!outputFeatures) return "";

            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            MLFeatureValue* value = [outputFeatures featureValueForName:featureName];

            if (value && value.type == MLFeatureTypeString) {
                return [value.stringValue UTF8String];
            }

            return "";
        }
    }

    ofPixels getImageOutput(const std::string& name) const {
        @autoreleasepool {
            ofPixels result;

            if (!outputFeatures) return result;

            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            MLFeatureValue* value = [outputFeatures featureValueForName:featureName];

            if (value && value.type == MLFeatureTypeImage) {
                CVPixelBufferRef pixelBuffer = value.imageBufferValue;
                if (!pixelBuffer) return result;

                CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

                size_t width = CVPixelBufferGetWidth(pixelBuffer);
                size_t height = CVPixelBufferGetHeight(pixelBuffer);
                size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);
                void* baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);

                // Allocate ofPixels
                result.allocate(width, height, OF_PIXELS_RGBA);

                // Copy data (assuming BGRA format from Core ML)
                uint8_t* src = (uint8_t*)baseAddress;
                uint8_t* dst = result.getData();

                for (size_t y = 0; y < height; y++) {
                    uint8_t* srcRow = src + y * bytesPerRow;
                    uint8_t* dstRow = dst + y * width * 4;

                    for (size_t x = 0; x < width; x++) {
                        // Convert BGRA to RGBA
                        dstRow[x * 4 + 0] = srcRow[x * 4 + 2]; // R
                        dstRow[x * 4 + 1] = srcRow[x * 4 + 1]; // G
                        dstRow[x * 4 + 2] = srcRow[x * 4 + 0]; // B
                        dstRow[x * 4 + 3] = srcRow[x * 4 + 3]; // A
                    }
                }

                CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
            }

            return result;
        }
    }

    std::vector<float> getArrayOutput(const std::string& name) const {
        @autoreleasepool {
            std::vector<float> result;

            if (!outputFeatures) return result;

            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            MLFeatureValue* value = [outputFeatures featureValueForName:featureName];

            if (value && value.type == MLFeatureTypeMultiArray) {
                MLMultiArray* multiArray = value.multiArrayValue;
                if (!multiArray) return result;

                // Calculate total number of elements
                size_t totalElements = 1;
                for (NSNumber* dim in multiArray.shape) {
                    totalElements *= [dim unsignedLongValue];
                }

                result.resize(totalElements);

                // Copy data based on type
                if (multiArray.dataType == MLMultiArrayDataTypeFloat32) {
                    float* srcData = (float*)multiArray.dataPointer;
                    std::copy(srcData, srcData + totalElements, result.begin());
                } else if (multiArray.dataType == MLMultiArrayDataTypeDouble) {
                    double* srcData = (double*)multiArray.dataPointer;
                    for (size_t i = 0; i < totalElements; i++) {
                        result[i] = static_cast<float>(srcData[i]);
                    }
                } else if (multiArray.dataType == MLMultiArrayDataTypeInt32) {
                    int32_t* srcData = (int32_t*)multiArray.dataPointer;
                    for (size_t i = 0; i < totalElements; i++) {
                        result[i] = static_cast<float>(srcData[i]);
                    }
                }
            }

            return result;
        }
    }

    std::pair<std::vector<float>, std::vector<size_t>> getArrayOutputWithShape(const std::string& name) const {
        @autoreleasepool {
            std::vector<float> data;
            std::vector<size_t> shape;

            if (!outputFeatures) return {data, shape};

            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            MLFeatureValue* value = [outputFeatures featureValueForName:featureName];

            if (value && value.type == MLFeatureTypeMultiArray) {
                MLMultiArray* multiArray = value.multiArrayValue;
                if (!multiArray) return {data, shape};

                // Extract shape
                for (NSNumber* dim in multiArray.shape) {
                    shape.push_back([dim unsignedLongValue]);
                }

                // Get data
                data = getArrayOutput(name);
            }

            return {data, shape};
        }
    }

    std::map<std::string, double> getDictionaryOutput(const std::string& name) const {
        @autoreleasepool {
            std::map<std::string, double> result;

            if (!outputFeatures) return result;

            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            MLFeatureValue* value = [outputFeatures featureValueForName:featureName];

            if (value && value.type == MLFeatureTypeDictionary) {
                NSDictionary* dict = value.dictionaryValue;

                for (id key in dict) {
                    if ([key isKindOfClass:[NSString class]]) {
                        NSString* nsKey = (NSString*)key;
                        id obj = dict[key];

                        double doubleValue = 0.0;
                        if ([obj isKindOfClass:[NSNumber class]]) {
                            doubleValue = [(NSNumber*)obj doubleValue];
                        }

                        result[[nsKey UTF8String]] = doubleValue;
                    }
                }
            }

            return result;
        }
    }

    std::vector<size_t> getOutputShape(const std::string& name) const {
        @autoreleasepool {
            std::vector<size_t> shape;

            if (!outputFeatures) return shape;

            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            MLFeatureValue* value = [outputFeatures featureValueForName:featureName];

            if (value && value.type == MLFeatureTypeMultiArray) {
                MLMultiArray* multiArray = value.multiArrayValue;
                if (multiArray) {
                    for (NSNumber* dim in multiArray.shape) {
                        shape.push_back([dim unsignedLongValue]);
                    }
                }
            }

            return shape;
        }
    }

    bool hasOutput(const std::string& name) const {
        @autoreleasepool {
            if (!outputFeatures) return false;

            NSString* featureName = [NSString stringWithUTF8String:name.c_str()];
            MLFeatureValue* value = [outputFeatures featureValueForName:featureName];
            return (value != nil);
        }
    }

    std::vector<std::string> getOutputNames() const {
        @autoreleasepool {
            std::vector<std::string> names;

            if (!outputFeatures) return names;

            NSSet<NSString*>* featureNames = outputFeatures.featureNames;
            for (NSString* name in featureNames) {
                names.push_back([name UTF8String]);
            }

            return names;
        }
    }
};

// ============================================================================
// Public API Implementation
// ============================================================================

GenericModel::GenericModel() : impl_(std::make_unique<Impl>()) {}
GenericModel::~GenericModel() = default;

GenericModel::GenericModel(GenericModel&& other) noexcept = default;
GenericModel& GenericModel::operator=(GenericModel&& other) noexcept = default;

bool GenericModel::load(const std::string& modelPath) {
    GenericModelConfig config;
    return impl_->loadModel(modelPath, config);
}

bool GenericModel::load(const std::string& modelPath, const GenericModelConfig& config) {
    return impl_->loadModel(modelPath, config);
}

void GenericModel::unload() {
    impl_->model = nil;
    impl_->outputFeatures = nil;
    impl_->info = GenericModelInfo();
    impl_->clearInputs();
}

// Input setters
void GenericModel::setInput(const std::string& name, const ofPixels& pixels) {
    impl_->setImageInput(name, pixels);
}

void GenericModel::setInput(const std::string& name, const ofTexture& texture) {
    // DEFERRED: Metal direct path optimization
    //
    // Rationale:
    // - MLFeatureValue expects CVPixelBuffer or CGImage
    // - readToPixels() provides necessary conversion via Metal readback
    // - Direct MTLTexture → CVPixelBuffer requires custom Metal compute
    // - Current path is sufficient for generic model inputs
    //
    // Convert texture to pixels
    ofPixels pixels;
    texture.readToPixels(pixels);
    impl_->setImageInput(name, pixels);
}

void GenericModel::setInput(const std::string& name, int64_t value) {
    impl_->setInt64Input(name, value);
}

void GenericModel::setInput(const std::string& name, double value) {
    impl_->setDoubleInput(name, value);
}

void GenericModel::setInput(const std::string& name, float value) {
    impl_->setDoubleInput(name, static_cast<double>(value));
}

void GenericModel::setInput(const std::string& name, int value) {
    impl_->setInt64Input(name, static_cast<int64_t>(value));
}

void GenericModel::setInput(const std::string& name, const std::string& value) {
    impl_->setStringInput(name, value);
}

void GenericModel::setInput(const std::string& name, const std::vector<float>& data,
                           const std::vector<size_t>& shape) {
    impl_->setMultiArrayInput(name, data.data(), shape, MLMultiArrayDataTypeFloat32);
}

void GenericModel::setInput(const std::string& name, const std::vector<double>& data,
                           const std::vector<size_t>& shape) {
    impl_->setMultiArrayInput(name, data.data(), shape, MLMultiArrayDataTypeDouble);
}

void GenericModel::setInput(const std::string& name, const std::vector<int32_t>& data,
                           const std::vector<size_t>& shape) {
    impl_->setMultiArrayInput(name, data.data(), shape, MLMultiArrayDataTypeInt32);
}

void GenericModel::setInput(const std::string& name, const std::map<std::string, double>& dict) {
    impl_->setDictionaryInput(name, dict);
}

void GenericModel::clearInputs() {
    @autoreleasepool {
        [impl_->inputFeatures removeAllObjects];
    }
}

// Prediction
bool GenericModel::predict() {
    return impl_->runPrediction();
}

bool GenericModel::predictBatch(const std::vector<std::map<std::string, std::any>>& batchInputs) {
    // DEFERRED: Batch prediction support
    //
    // Rationale:
    // - Requires MLBatchProvider implementation with array of feature providers
    // - Core ML batch API: [MLModel predictionFromBatch:error:]
    // - Single prediction API is sufficient for most real-time use cases
    // - Batch prediction benefits server/offline processing (not primary use case)
    //
    // Implementation notes for future:
    // 1. Create custom class conforming to MLBatchProvider protocol
    // 2. Provide count and featuresAtIndex: methods
    // 3. Call [model predictionFromBatch:batchProvider error:]
    // 4. Extract results from MLBatchProvider output

    impl_->lastError = "Batch prediction not yet implemented";
    return false;
}

// Output getters
int64_t GenericModel::getOutputInt64(const std::string& name) const {
    return impl_->getInt64Output(name);
}

double GenericModel::getOutputDouble(const std::string& name) const {
    return impl_->getDoubleOutput(name);
}

float GenericModel::getOutputFloat(const std::string& name) const {
    return static_cast<float>(impl_->getDoubleOutput(name));
}

std::string GenericModel::getOutputString(const std::string& name) const {
    return impl_->getStringOutput(name);
}

ofPixels GenericModel::getOutputImage(const std::string& name) const {
    return impl_->getImageOutput(name);
}

std::vector<float> GenericModel::getOutputArray(const std::string& name) const {
    return impl_->getArrayOutput(name);
}

std::pair<std::vector<float>, std::vector<size_t>> GenericModel::getOutputArrayWithShape(const std::string& name) const {
    return impl_->getArrayOutputWithShape(name);
}

std::map<std::string, double> GenericModel::getOutputDictionary(const std::string& name) const {
    return impl_->getDictionaryOutput(name);
}

std::vector<size_t> GenericModel::getOutputShape(const std::string& name) const {
    return impl_->getOutputShape(name);
}

bool GenericModel::hasOutput(const std::string& name) const {
    return impl_->hasOutput(name);
}

std::vector<std::string> GenericModel::getOutputNames() const {
    return impl_->getOutputNames();
}

// Information
bool GenericModel::isLoaded() const {
    return impl_->info.isLoaded;
}

GenericModelInfo GenericModel::getInfo() const {
    return impl_->info;
}

FeatureInfo GenericModel::getInputInfo(const std::string& name) const {
    for (const auto& info : impl_->info.inputs) {
        if (info.name == name) {
            return info;
        }
    }
    return FeatureInfo();
}

FeatureInfo GenericModel::getOutputInfo(const std::string& name) const {
    for (const auto& info : impl_->info.outputs) {
        if (info.name == name) {
            return info;
        }
    }
    return FeatureInfo();
}

std::vector<std::string> GenericModel::getInputNames() const {
    std::vector<std::string> names;
    for (const auto& info : impl_->info.inputs) {
        names.push_back(info.name);
    }
    return names;
}

std::string GenericModel::getLastError() const {
    return impl_->lastError;
}

void* GenericModel::getNativeHandle() const {
    return (__bridge void*)impl_->model;
}

} // namespace NeuralEngine

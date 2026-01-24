#include "ofxGui.h"
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

// Forward declaration of Swift class
// OFLGuiPanel.swift exports GuiParameterStore to Objective-C
@class GuiParameterStore;
@class GuiFloatParameter;
@class GuiIntParameter;
@class GuiBoolParameter;
@class GuiColorParameter;
@class GuiButtonParameter;
@class GuiStringParameter;

// Access Swift's GuiParameterStore singleton
extern "C" {
    // These functions are provided by Swift runtime
    // GuiParameterStore.shared is accessible from Objective-C++
}

// Helper to get GuiParameterStore.shared
static id getGuiParameterStore() {
    @autoreleasepool {
        // Access Swift's GuiParameterStore.shared singleton
        Class storeClass = NSClassFromString(@"oflike_metal.GuiParameterStore");
        if (!storeClass) {
            storeClass = NSClassFromString(@"GuiParameterStore");
        }
        if (storeClass) {
            SEL sharedSelector = NSSelectorFromString(@"shared");
            if ([storeClass respondsToSelector:sharedSelector]) {
                #pragma clang diagnostic push
                #pragma clang diagnostic ignored "-Warc-performSelector-leaks"
                return [storeClass performSelector:sharedSelector];
                #pragma clang diagnostic pop
            }
        }
        return nil;
    }
}

// MARK: - ofxGuiGroup Implementation

class ofxGuiGroup::Impl {
public:
    std::string name;

    Impl() : name("General") {}

    explicit Impl(const std::string& n) : name(n) {}
};

ofxGuiGroup::ofxGuiGroup() : impl_(std::make_unique<Impl>()) {}

ofxGuiGroup::ofxGuiGroup(const std::string& name) : impl_(std::make_unique<Impl>(name)) {}

ofxGuiGroup::~ofxGuiGroup() = default;

void ofxGuiGroup::setName(const std::string& name) {
    impl_->name = name;
}

std::string ofxGuiGroup::getName() const {
    return impl_->name;
}

void ofxGuiGroup::addSlider(const std::string& name, float& value, float min, float max) {
    @autoreleasepool {
        id store = getGuiParameterStore();
        if (store) {
            NSString* nsName = [NSString stringWithUTF8String:name.c_str()];
            NSString* nsGroup = [NSString stringWithUTF8String:impl_->name.c_str()];

            SEL selector = NSSelectorFromString(@"registerFloatWithName:group:value:min:max:");
            if ([store respondsToSelector:selector]) {
                NSMethodSignature* signature = [store methodSignatureForSelector:selector];
                NSInvocation* invocation = [NSInvocation invocationWithMethodSignature:signature];
                [invocation setSelector:selector];
                [invocation setTarget:store];
                [invocation setArgument:&nsName atIndex:2];
                [invocation setArgument:&nsGroup atIndex:3];
                [invocation setArgument:&value atIndex:4];
                [invocation setArgument:&min atIndex:5];
                [invocation setArgument:&max atIndex:6];
                [invocation invoke];

                // TODO: Store parameter and update value binding
            }
        }
    }
}

void ofxGuiGroup::addSlider(const std::string& name, int& value, int min, int max) {
    @autoreleasepool {
        id store = getGuiParameterStore();
        if (store) {
            NSString* nsName = [NSString stringWithUTF8String:name.c_str()];
            NSString* nsGroup = [NSString stringWithUTF8String:impl_->name.c_str()];

            SEL selector = NSSelectorFromString(@"registerIntWithName:group:value:min:max:");
            if ([store respondsToSelector:selector]) {
                NSMethodSignature* signature = [store methodSignatureForSelector:selector];
                NSInvocation* invocation = [NSInvocation invocationWithMethodSignature:signature];
                [invocation setSelector:selector];
                [invocation setTarget:store];
                [invocation setArgument:&nsName atIndex:2];
                [invocation setArgument:&nsGroup atIndex:3];
                [invocation setArgument:&value atIndex:4];
                [invocation setArgument:&min atIndex:5];
                [invocation setArgument:&max atIndex:6];
                [invocation invoke];
            }
        }
    }
}

void ofxGuiGroup::addToggle(const std::string& name, bool& value) {
    @autoreleasepool {
        id store = getGuiParameterStore();
        if (store) {
            NSString* nsName = [NSString stringWithUTF8String:name.c_str()];
            NSString* nsGroup = [NSString stringWithUTF8String:impl_->name.c_str()];

            SEL selector = NSSelectorFromString(@"registerBoolWithName:group:value:");
            if ([store respondsToSelector:selector]) {
                NSMethodSignature* signature = [store methodSignatureForSelector:selector];
                NSInvocation* invocation = [NSInvocation invocationWithMethodSignature:signature];
                [invocation setSelector:selector];
                [invocation setTarget:store];
                [invocation setArgument:&nsName atIndex:2];
                [invocation setArgument:&nsGroup atIndex:3];
                [invocation setArgument:&value atIndex:4];
                [invocation invoke];
            }
        }
    }
}

void ofxGuiGroup::addButton(const std::string& name, std::function<void()> callback) {
    @autoreleasepool {
        id store = getGuiParameterStore();
        if (store) {
            NSString* nsName = [NSString stringWithUTF8String:name.c_str()];
            NSString* nsGroup = [NSString stringWithUTF8String:impl_->name.c_str()];

            // Create callback block
            void (^block)() = ^{
                if (callback) {
                    callback();
                }
            };

            SEL selector = NSSelectorFromString(@"registerButtonWithName:group:action:");
            if ([store respondsToSelector:selector]) {
                NSMethodSignature* signature = [store methodSignatureForSelector:selector];
                NSInvocation* invocation = [NSInvocation invocationWithMethodSignature:signature];
                [invocation setSelector:selector];
                [invocation setTarget:store];
                [invocation setArgument:&nsName atIndex:2];
                [invocation setArgument:&nsGroup atIndex:3];
                [invocation setArgument:&block atIndex:4];
                [invocation invoke];
            }
        }
    }
}

void ofxGuiGroup::addColor(const std::string& name, ofColor& color) {
    @autoreleasepool {
        id store = getGuiParameterStore();
        if (store) {
            NSString* nsName = [NSString stringWithUTF8String:name.c_str()];
            NSString* nsGroup = [NSString stringWithUTF8String:impl_->name.c_str()];

            int r = color.r;
            int g = color.g;
            int b = color.b;

            SEL selector = NSSelectorFromString(@"registerColorWithName:group:r:g:b:");
            if ([store respondsToSelector:selector]) {
                NSMethodSignature* signature = [store methodSignatureForSelector:selector];
                NSInvocation* invocation = [NSInvocation invocationWithMethodSignature:signature];
                [invocation setSelector:selector];
                [invocation setTarget:store];
                [invocation setArgument:&nsName atIndex:2];
                [invocation setArgument:&nsGroup atIndex:3];
                [invocation setArgument:&r atIndex:4];
                [invocation setArgument:&g atIndex:5];
                [invocation setArgument:&b atIndex:6];
                [invocation invoke];
            }
        }
    }
}

void ofxGuiGroup::addLabel(const std::string& name, std::string& value) {
    @autoreleasepool {
        id store = getGuiParameterStore();
        if (store) {
            NSString* nsName = [NSString stringWithUTF8String:name.c_str()];
            NSString* nsGroup = [NSString stringWithUTF8String:impl_->name.c_str()];
            NSString* nsValue = [NSString stringWithUTF8String:value.c_str()];

            SEL selector = NSSelectorFromString(@"registerStringWithName:group:value:");
            if ([store respondsToSelector:selector]) {
                NSMethodSignature* signature = [store methodSignatureForSelector:selector];
                NSInvocation* invocation = [NSInvocation invocationWithMethodSignature:signature];
                [invocation setSelector:selector];
                [invocation setTarget:store];
                [invocation setArgument:&nsName atIndex:2];
                [invocation setArgument:&nsGroup atIndex:3];
                [invocation setArgument:&nsValue atIndex:4];
                [invocation invoke];
            }
        }
    }
}

// MARK: - ofxPanel Implementation

class ofxPanel::Impl {
public:
    std::string name;
    std::vector<ofxGuiGroup*> groups;

    Impl() : name("GUI Panel") {}

    ~Impl() {
        // Don't delete groups - they're managed by user code
    }
};

ofxPanel::ofxPanel() : impl_(std::make_unique<Impl>()) {}

ofxPanel::~ofxPanel() = default;

void ofxPanel::setup(const std::string& name) {
    impl_->name = name;
}

void ofxPanel::add(ofxGuiGroup& group) {
    impl_->groups.push_back(&group);
}

void ofxPanel::addSlider(const std::string& name, float& value, float min, float max) {
    ofxGuiGroup defaultGroup;
    defaultGroup.addSlider(name, value, min, max);
}

void ofxPanel::addSlider(const std::string& name, int& value, int min, int max) {
    ofxGuiGroup defaultGroup;
    defaultGroup.addSlider(name, value, min, max);
}

void ofxPanel::addToggle(const std::string& name, bool& value) {
    ofxGuiGroup defaultGroup;
    defaultGroup.addToggle(name, value);
}

void ofxPanel::addButton(const std::string& name, std::function<void()> callback) {
    ofxGuiGroup defaultGroup;
    defaultGroup.addButton(name, callback);
}

void ofxPanel::addColor(const std::string& name, ofColor& color) {
    ofxGuiGroup defaultGroup;
    defaultGroup.addColor(name, color);
}

void ofxPanel::addLabel(const std::string& name, std::string& value) {
    ofxGuiGroup defaultGroup;
    defaultGroup.addLabel(name, value);
}

std::string ofxPanel::getName() const {
    return impl_->name;
}

void ofxPanel::clear() {
    @autoreleasepool {
        id store = getGuiParameterStore();
        if (store) {
            SEL selector = NSSelectorFromString(@"clear");
            if ([store respondsToSelector:selector]) {
                #pragma clang diagnostic push
                #pragma clang diagnostic ignored "-Warc-performSelector-leaks"
                [store performSelector:selector];
                #pragma clang diagnostic pop
            }
        }
    }
    impl_->groups.clear();
}

// MARK: - Convenience Type Implementations

// ofxFloatSlider
class ofxFloatSlider::Impl {
public:
    float* valuePtr = nullptr;
    float localValue = 0.0f;
};

ofxFloatSlider::ofxFloatSlider() : impl_(std::make_unique<Impl>()) {}

ofxFloatSlider::ofxFloatSlider(const std::string& name, float value, float min, float max, const std::string& group)
    : impl_(std::make_unique<Impl>()) {
    setup(name, value, min, max, group);
}

ofxFloatSlider::~ofxFloatSlider() = default;

void ofxFloatSlider::setup(const std::string& name, float value, float min, float max, const std::string& group) {
    impl_->localValue = value;
    impl_->valuePtr = &impl_->localValue;
    ofxGuiGroup grp(group);
    grp.addSlider(name, *impl_->valuePtr, min, max);
}

float ofxFloatSlider::getValue() const {
    return impl_->valuePtr ? *impl_->valuePtr : impl_->localValue;
}

void ofxFloatSlider::setValue(float value) {
    impl_->localValue = value;
    if (impl_->valuePtr) {
        *impl_->valuePtr = value;
    }
}

ofxFloatSlider::operator float() const {
    return getValue();
}

ofxFloatSlider& ofxFloatSlider::operator=(float value) {
    setValue(value);
    return *this;
}

// ofxIntSlider
class ofxIntSlider::Impl {
public:
    int* valuePtr = nullptr;
    int localValue = 0;
};

ofxIntSlider::ofxIntSlider() : impl_(std::make_unique<Impl>()) {}

ofxIntSlider::ofxIntSlider(const std::string& name, int value, int min, int max, const std::string& group)
    : impl_(std::make_unique<Impl>()) {
    setup(name, value, min, max, group);
}

ofxIntSlider::~ofxIntSlider() = default;

void ofxIntSlider::setup(const std::string& name, int value, int min, int max, const std::string& group) {
    impl_->localValue = value;
    impl_->valuePtr = &impl_->localValue;
    ofxGuiGroup grp(group);
    grp.addSlider(name, *impl_->valuePtr, min, max);
}

int ofxIntSlider::getValue() const {
    return impl_->valuePtr ? *impl_->valuePtr : impl_->localValue;
}

void ofxIntSlider::setValue(int value) {
    impl_->localValue = value;
    if (impl_->valuePtr) {
        *impl_->valuePtr = value;
    }
}

ofxIntSlider::operator int() const {
    return getValue();
}

ofxIntSlider& ofxIntSlider::operator=(int value) {
    setValue(value);
    return *this;
}

// ofxToggle
class ofxToggle::Impl {
public:
    bool* valuePtr = nullptr;
    bool localValue = false;
};

ofxToggle::ofxToggle() : impl_(std::make_unique<Impl>()) {}

ofxToggle::ofxToggle(const std::string& name, bool value, const std::string& group)
    : impl_(std::make_unique<Impl>()) {
    setup(name, value, group);
}

ofxToggle::~ofxToggle() = default;

void ofxToggle::setup(const std::string& name, bool value, const std::string& group) {
    impl_->localValue = value;
    impl_->valuePtr = &impl_->localValue;
    ofxGuiGroup grp(group);
    grp.addToggle(name, *impl_->valuePtr);
}

bool ofxToggle::getValue() const {
    return impl_->valuePtr ? *impl_->valuePtr : impl_->localValue;
}

void ofxToggle::setValue(bool value) {
    impl_->localValue = value;
    if (impl_->valuePtr) {
        *impl_->valuePtr = value;
    }
}

ofxToggle::operator bool() const {
    return getValue();
}

ofxToggle& ofxToggle::operator=(bool value) {
    setValue(value);
    return *this;
}

// ofxButton
class ofxButton::Impl {
public:
    std::function<void()> callback;
};

ofxButton::ofxButton() : impl_(std::make_unique<Impl>()) {}

ofxButton::ofxButton(const std::string& name, std::function<void()> callback, const std::string& group)
    : impl_(std::make_unique<Impl>()) {
    setup(name, callback, group);
}

ofxButton::~ofxButton() = default;

void ofxButton::setup(const std::string& name, std::function<void()> callback, const std::string& group) {
    impl_->callback = callback;
    ofxGuiGroup grp(group);
    grp.addButton(name, callback);
}

// ofxColorPicker
class ofxColorPicker::Impl {
public:
    ofColor* colorPtr = nullptr;
    ofColor localColor;
};

ofxColorPicker::ofxColorPicker() : impl_(std::make_unique<Impl>()) {}

ofxColorPicker::ofxColorPicker(const std::string& name, const ofColor& color, const std::string& group)
    : impl_(std::make_unique<Impl>()) {
    setup(name, color, group);
}

ofxColorPicker::~ofxColorPicker() = default;

void ofxColorPicker::setup(const std::string& name, const ofColor& color, const std::string& group) {
    impl_->localColor = color;
    impl_->colorPtr = &impl_->localColor;
    ofxGuiGroup grp(group);
    grp.addColor(name, *impl_->colorPtr);
}

ofColor ofxColorPicker::getColor() const {
    return impl_->colorPtr ? *impl_->colorPtr : impl_->localColor;
}

void ofxColorPicker::setColor(const ofColor& color) {
    impl_->localColor = color;
    if (impl_->colorPtr) {
        *impl_->colorPtr = color;
    }
}

ofxColorPicker::operator ofColor() const {
    return getColor();
}

ofxColorPicker& ofxColorPicker::operator=(const ofColor& color) {
    setColor(color);
    return *this;
}

// ofxLabel
class ofxLabel::Impl {
public:
    std::string* valuePtr = nullptr;
    std::string localValue;
};

ofxLabel::ofxLabel() : impl_(std::make_unique<Impl>()) {}

ofxLabel::ofxLabel(const std::string& name, const std::string& value, const std::string& group)
    : impl_(std::make_unique<Impl>()) {
    setup(name, value, group);
}

ofxLabel::~ofxLabel() = default;

void ofxLabel::setup(const std::string& name, const std::string& value, const std::string& group) {
    impl_->localValue = value;
    impl_->valuePtr = &impl_->localValue;
    ofxGuiGroup grp(group);
    grp.addLabel(name, *impl_->valuePtr);
}

std::string ofxLabel::getValue() const {
    return impl_->valuePtr ? *impl_->valuePtr : impl_->localValue;
}

void ofxLabel::setValue(const std::string& value) {
    impl_->localValue = value;
    if (impl_->valuePtr) {
        *impl_->valuePtr = value;
    }
}

ofxLabel::operator std::string() const {
    return getValue();
}

ofxLabel& ofxLabel::operator=(const std::string& value) {
    setValue(value);
    return *this;
}

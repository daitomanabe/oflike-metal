#pragma once

// ofParameter - Value container with change callbacks
// Used for GUI and OSC parameter synchronization

#include <string>
#include <functional>
#include <memory>
#include <vector>

// Forward declarations
class ofAbstractParameter;
template<typename T> class ofParameter;
class ofParameterGroup;

// MARK: - ofAbstractParameter

/// Base class for all parameters
class ofAbstractParameter {
public:
    virtual ~ofAbstractParameter() = default;

    virtual std::string getName() const = 0;
    virtual void setName(const std::string& name) = 0;

    virtual std::string getValueAsString() const = 0;
    virtual void setFromString(const std::string& value) = 0;

    virtual std::string getType() const = 0;
};

// MARK: - ofParameter<T>

/// Templated parameter with value storage and change callbacks
template<typename T>
class ofParameter : public ofAbstractParameter {
public:
    ofParameter() : value_(T()), min_(T()), max_(T()), name_("") {}

    ofParameter(const std::string& name, T value)
        : value_(value), min_(T()), max_(T()), name_(name) {}

    ofParameter(const std::string& name, T value, T min, T max)
        : value_(value), min_(min), max_(max), name_(name) {}

    // Value access
    T get() const { return value_; }

    void set(T value) {
        if (value_ != value) {
            value_ = value;
            notifyListeners();
        }
    }

    T getMin() const { return min_; }
    T getMax() const { return max_; }

    void setMin(T min) { min_ = min; }
    void setMax(T max) { max_ = max; }

    // Name
    std::string getName() const override { return name_; }
    void setName(const std::string& name) override { name_ = name; }

    // String conversion
    std::string getValueAsString() const override;
    void setFromString(const std::string& value) override;

    std::string getType() const override;

    // Operators
    operator T() const { return value_; }

    ofParameter<T>& operator=(T value) {
        set(value);
        return *this;
    }

    // Change listeners
    void addListener(std::function<void(T&)> listener) {
        listeners_.push_back(listener);
    }

    void removeListeners() {
        listeners_.clear();
    }

private:
    void notifyListeners() {
        for (auto& listener : listeners_) {
            listener(value_);
        }
    }

    T value_;
    T min_;
    T max_;
    std::string name_;
    std::vector<std::function<void(T&)>> listeners_;
};

// MARK: - ofParameterGroup

/// Group of parameters
class ofParameterGroup : public ofAbstractParameter {
public:
    ofParameterGroup();
    ofParameterGroup(const std::string& name);
    ~ofParameterGroup();

    // Name
    std::string getName() const override;
    void setName(const std::string& name) override;

    // String conversion (returns group info)
    std::string getValueAsString() const override;
    void setFromString(const std::string& value) override;

    std::string getType() const override { return "group"; }

    // Add parameters
    void add(std::shared_ptr<ofAbstractParameter> param);

    template<typename T>
    void add(ofParameter<T>& param) {
        add(std::make_shared<ofParameter<T>>(param));
    }

    // Access
    size_t size() const;
    std::shared_ptr<ofAbstractParameter> get(size_t index) const;
    std::shared_ptr<ofAbstractParameter> get(const std::string& name) const;

    void clear();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// MARK: - Type-specific implementations

// String conversion implementations
template<>
inline std::string ofParameter<float>::getValueAsString() const {
    return std::to_string(value_);
}

template<>
inline std::string ofParameter<int>::getValueAsString() const {
    return std::to_string(value_);
}

template<>
inline std::string ofParameter<bool>::getValueAsString() const {
    return value_ ? "true" : "false";
}

template<>
inline std::string ofParameter<std::string>::getValueAsString() const {
    return value_;
}

template<>
inline void ofParameter<float>::setFromString(const std::string& value) {
    set(std::stof(value));
}

template<>
inline void ofParameter<int>::setFromString(const std::string& value) {
    set(std::stoi(value));
}

template<>
inline void ofParameter<bool>::setFromString(const std::string& value) {
    set(value == "true" || value == "1");
}

template<>
inline void ofParameter<std::string>::setFromString(const std::string& value) {
    set(value);
}

// Type names
template<>
inline std::string ofParameter<float>::getType() const {
    return "float";
}

template<>
inline std::string ofParameter<int>::getType() const {
    return "int";
}

template<>
inline std::string ofParameter<bool>::getType() const {
    return "bool";
}

template<>
inline std::string ofParameter<std::string>::getType() const {
    return "string";
}

#pragma once

// oflike-metal ofShader - openFrameworks API compatible shader management
// Loads and manages custom Metal shaders with uniform support

#include <memory>
#include <string>
#include "../math/ofVec2f.h"
#include "../math/ofVec3f.h"
#include "../math/ofVec4f.h"
#include "../math/ofMatrix4x4.h"
#include "../types/ofColor.h"

namespace oflike {

// Forward declarations
class ofTexture;

/// \brief Custom Metal shader management
/// \details ofShader provides openFrameworks-compatible shader loading and
/// parameter setting for Metal shaders. Supports loading from .metal source files
/// or pre-compiled .metallib files.
///
/// Features:
/// - Load shaders from .metal source or .metallib
/// - Set uniform values (float, vec, matrix, texture)
/// - begin/end for shader activation
/// - Automatic uniform buffer management
///
/// Note: Unlike OpenGL shaders, Metal shaders are compiled to .metallib at
/// build time for optimal performance. Runtime compilation is also supported
/// for development/hot-reload scenarios.
///
/// Example:
/// \code
///     ofShader shader;
///     shader.load("shaders/custom");  // Loads custom.metal or from metallib
///
///     shader.begin();
///     shader.setUniform1f("time", ofGetElapsedTimef());
///     shader.setUniform3f("lightPos", 100, 200, 300);
///     shader.setUniformMatrix4f("modelMatrix", mesh.getTransformMatrix());
///     // Draw content here
///     shader.end();
/// \endcode
class ofShader {
public:
    // ========================================================================
    // Construction / Destruction
    // ========================================================================

    /// \brief Default constructor
    ofShader();

    /// \brief Destructor
    ~ofShader();

    /// \brief Move constructor
    ofShader(ofShader&& other) noexcept;

    /// \brief Move assignment
    ofShader& operator=(ofShader&& other) noexcept;

    // Delete copy
    ofShader(const ofShader&) = delete;
    ofShader& operator=(const ofShader&) = delete;

    // ========================================================================
    // Loading
    // ========================================================================

    /// \brief Load shader from file
    /// \param shaderName Base name without extension (e.g., "shaders/custom")
    /// \return true if loaded successfully
    /// \details Tries to load in this order:
    /// 1. Pre-compiled: {shaderName}.metallib
    /// 2. Source: {shaderName}.metal
    /// Default function names: vertex_main, fragment_main
    bool load(const std::string& shaderName);

    /// \brief Load shader with explicit function names
    /// \param shaderName Base name without extension
    /// \param vertexFunctionName Name of vertex function in shader
    /// \param fragmentFunctionName Name of fragment function in shader
    /// \return true if loaded successfully
    bool load(const std::string& shaderName,
              const std::string& vertexFunctionName,
              const std::string& fragmentFunctionName);

    /// \brief Load shader from source string
    /// \param source Metal shader source code
    /// \param vertexFunctionName Name of vertex function
    /// \param fragmentFunctionName Name of fragment function
    /// \return true if loaded successfully
    bool loadFromSource(const std::string& source,
                        const std::string& vertexFunctionName = "vertex_main",
                        const std::string& fragmentFunctionName = "fragment_main");

    /// \brief Unload shader and free resources
    void unload();

    /// \brief Check if shader is loaded
    /// \return true if shader is ready to use
    bool isLoaded() const;

    // ========================================================================
    // Activation
    // ========================================================================

    /// \brief Begin using this shader
    /// \details All subsequent draw calls will use this shader until end() is called.
    void begin();

    /// \brief End using this shader
    /// \details Restores default shader.
    void end();

    /// \brief Check if shader is currently active
    /// \return true if between begin() and end()
    bool isActive() const;

    // ========================================================================
    // Uniform Setters - Scalars
    // ========================================================================

    /// \brief Set float uniform
    /// \param name Uniform name
    /// \param v Value
    void setUniform1f(const std::string& name, float v);

    /// \brief Set int uniform
    /// \param name Uniform name
    /// \param v Value
    void setUniform1i(const std::string& name, int v);

    /// \brief Set bool uniform (as int)
    /// \param name Uniform name
    /// \param v Value
    void setUniform1b(const std::string& name, bool v);

    // ========================================================================
    // Uniform Setters - Vectors
    // ========================================================================

    /// \brief Set vec2 uniform
    /// \param name Uniform name
    /// \param x X component
    /// \param y Y component
    void setUniform2f(const std::string& name, float x, float y);

    /// \brief Set vec2 uniform from vector
    /// \param name Uniform name
    /// \param v Vector value
    void setUniform2f(const std::string& name, const ofVec2f& v);

    /// \brief Set vec3 uniform
    /// \param name Uniform name
    /// \param x X component
    /// \param y Y component
    /// \param z Z component
    void setUniform3f(const std::string& name, float x, float y, float z);

    /// \brief Set vec3 uniform from vector
    /// \param name Uniform name
    /// \param v Vector value
    void setUniform3f(const std::string& name, const ofVec3f& v);

    /// \brief Set vec4 uniform
    /// \param name Uniform name
    /// \param x X component
    /// \param y Y component
    /// \param z Z component
    /// \param w W component
    void setUniform4f(const std::string& name, float x, float y, float z, float w);

    /// \brief Set vec4 uniform from vector
    /// \param name Uniform name
    /// \param v Vector value
    void setUniform4f(const std::string& name, const ofVec4f& v);

    /// \brief Set vec4 uniform from color
    /// \param name Uniform name
    /// \param c Color value (normalized 0-1)
    void setUniform4f(const std::string& name, const ofFloatColor& c);

    // ========================================================================
    // Uniform Setters - Integer Vectors
    // ========================================================================

    /// \brief Set ivec2 uniform
    void setUniform2i(const std::string& name, int x, int y);

    /// \brief Set ivec3 uniform
    void setUniform3i(const std::string& name, int x, int y, int z);

    /// \brief Set ivec4 uniform
    void setUniform4i(const std::string& name, int x, int y, int z, int w);

    // ========================================================================
    // Uniform Setters - Matrices
    // ========================================================================

    /// \brief Set mat3 uniform (3x3 matrix)
    /// \param name Uniform name
    /// \param m Matrix data (column-major, 9 floats)
    void setUniformMatrix3f(const std::string& name, const float* m);

    /// \brief Set mat4 uniform (4x4 matrix)
    /// \param name Uniform name
    /// \param m Matrix data (column-major, 16 floats)
    void setUniformMatrix4f(const std::string& name, const float* m);

    /// \brief Set mat4 uniform from ofMatrix4x4
    /// \param name Uniform name
    /// \param m Matrix value
    void setUniformMatrix4f(const std::string& name, const ofMatrix4x4& m);

    // ========================================================================
    // Uniform Setters - Arrays
    // ========================================================================

    /// \brief Set float array uniform
    /// \param name Uniform name
    /// \param values Array of floats
    /// \param count Number of elements
    void setUniform1fv(const std::string& name, const float* values, size_t count);

    /// \brief Set vec2 array uniform
    /// \param name Uniform name
    /// \param values Array of vec2 (2 floats each)
    /// \param count Number of vec2 elements
    void setUniform2fv(const std::string& name, const float* values, size_t count);

    /// \brief Set vec3 array uniform
    /// \param name Uniform name
    /// \param values Array of vec3 (3 floats each)
    /// \param count Number of vec3 elements
    void setUniform3fv(const std::string& name, const float* values, size_t count);

    /// \brief Set vec4 array uniform
    /// \param name Uniform name
    /// \param values Array of vec4 (4 floats each)
    /// \param count Number of vec4 elements
    void setUniform4fv(const std::string& name, const float* values, size_t count);

    // ========================================================================
    // Texture Setters
    // ========================================================================

    /// \brief Set texture uniform
    /// \param name Uniform name (sampler name in shader)
    /// \param texture Texture to bind
    /// \param textureIndex Texture unit index (0-15)
    void setUniformTexture(const std::string& name, const ofTexture& texture, int textureIndex = 0);

    /// \brief Set texture uniform from native handle
    /// \param name Uniform name
    /// \param textureHandle Native texture handle (id<MTLTexture>)
    /// \param textureIndex Texture unit index
    void setUniformTexture(const std::string& name, void* textureHandle, int textureIndex = 0);

    // ========================================================================
    // Information
    // ========================================================================

    /// \brief Get vertex function name
    /// \return Name of vertex function
    std::string getVertexFunctionName() const;

    /// \brief Get fragment function name
    /// \return Name of fragment function
    std::string getFragmentFunctionName() const;

    /// \brief Get shader source path
    /// \return Path used to load shader
    std::string getShaderPath() const;

    // ========================================================================
    // Native Access (Advanced)
    // ========================================================================

    /// \brief Get native render pipeline state
    /// \return id<MTLRenderPipelineState> or nullptr
    void* getNativePipelineState() const;

    /// \brief Get native vertex function
    /// \return id<MTLFunction> or nullptr
    void* getNativeVertexFunction() const;

    /// \brief Get native fragment function
    /// \return id<MTLFunction> or nullptr
    void* getNativeFragmentFunction() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    void ensureImpl();
};

} // namespace oflike

#pragma once

#include <memory>
#include "../Types.h"

namespace oflike {

class DrawList;

// MetalRenderer: Metal-backed 2D renderer with a C++ public header.
// Implementation is in Objective-C++ (*.mm) via pImpl.
// Design rule: no Objective-C types in public headers.
class MetalRenderer {
public:
  MetalRenderer();
  ~MetalRenderer();

  MetalRenderer(const MetalRenderer&) = delete;
  MetalRenderer& operator=(const MetalRenderer&) = delete;

  // Attach renderer to an MTKView (passed as opaque pointer)
  bool attachToView(void* mtkView);

  void setClearColor(Color4f c);

  // Begin a frame using MTKView-provided pass descriptor and drawable
  void beginFrame(void* renderPassDescriptor, void* drawable, int pixelW, int pixelH, float contentScale = 1.0f);
  void draw(const DrawList& list);
  void endFrame();

  // Get Metal device (opaque pointer to id<MTLDevice>)
  void* getDevice() const;

  // Draw a textured quad
  void drawTexture(void* texture, void* sampler, float x, float y, float w, float h);

  // Texture binding for mesh rendering
  void bindTexture(void* texture, void* sampler, int textureLocation = 0);
  void unbindTexture(int textureLocation = 0);

  // Get current content scale
  float getContentScale() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace oflike

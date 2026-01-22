#pragma once

#include <memory>
#include "DrawList2D.hpp"

namespace oflike {

// Renderer2D: Metal-backed renderer with a C++ public header.
// Implementation is expected to be in an Objective-C++ (*.mm) file via pImpl.
//
// Design rule: no Objective-C types in public headers.
class Renderer2D {
public:
  Renderer2D();
  ~Renderer2D();

  Renderer2D(const Renderer2D&) = delete;
  Renderer2D& operator=(const Renderer2D&) = delete;

  // Attach renderer to an MTKView.
  // The view is passed as an opaque pointer to avoid ObjC types in headers.
  bool attachToView(void* mtkView);

  void setClearColor(Color4f c);

  // Begin a frame using MTKView-provided pass descriptor and drawable.
  // pixelW/pixelH are the view drawableSize in pixels.
  // contentScale is the backing scale factor (2.0 for Retina).
  void beginFrame(void* renderPassDescriptor, void* drawable, int pixelW, int pixelH, float contentScale = 1.0f);
  void draw(const DrawList2D& list);
  void endFrame();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace oflike

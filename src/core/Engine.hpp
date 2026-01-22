#pragma once

#include <memory>

#include "AppBase.hpp"
#include "../render/DrawList2D.hpp"
#include "../render/Renderer2D.hpp"

namespace oflike {

// Engine owns the app lifecycle and the per-frame draw recording.
class Engine {
public:
  void setApp(std::unique_ptr<AppBase> app) { app_ = std::move(app); }
  AppBase* app() const { return app_.get(); }

  bool attachToView(void* mtkView) { return renderer_.attachToView(mtkView); }

  void setDrawableSizePixels(int w, int h) {
    pixelW_ = w;
    pixelH_ = h;
  }

  void setContentScale(float scale) { contentScale_ = scale; }

  void setClearColor(Color4f c) { renderer_.setClearColor(c); }

  // The platform layer calls tick() once per frame.
  // - dtSeconds: time since last frame
  // - renderPassDescriptor: MTKView.currentRenderPassDescriptor
  // - drawable: MTKView.currentDrawable
  void tick(double dtSeconds, void* renderPassDescriptor, void* drawable) {
    if (!app_) return;

    lastDt_ = dtSeconds;
    elapsed_ += dtSeconds;
    frameNum_++;

    // Update frame rate (exponential moving average)
    if (dtSeconds > 0) {
      float instantFps = 1.0f / static_cast<float>(dtSeconds);
      frameRate_ = frameRate_ * 0.9f + instantFps * 0.1f;
    }

    if (!didSetup_) {
      app_->setup();
      didSetup_ = true;
    }

    // Record draw commands
    drawList_.reset();

    app_->update();

    renderer_.beginFrame(renderPassDescriptor, drawable, pixelW_, pixelH_, contentScale_);
    app_->draw();
    renderer_.draw(drawList_);
    renderer_.endFrame();
  }

  double elapsedTimeSeconds() const { return elapsed_; }
  double lastFrameTimeSeconds() const { return lastDt_; }

  // Window size in points (not pixels)
  int getWindowWidth() const {
    return contentScale_ > 0 ? static_cast<int>(pixelW_ / contentScale_) : pixelW_;
  }

  int getWindowHeight() const {
    return contentScale_ > 0 ? static_cast<int>(pixelH_ / contentScale_) : pixelH_;
  }

  // Pixel dimensions (for Retina displays)
  int getPixelWidth() const { return pixelW_; }
  int getPixelHeight() const { return pixelH_; }

  float getFrameRate() const { return frameRate_; }
  int getFrameNum() const { return frameNum_; }

  DrawList2D& drawList() { return drawList_; }
  Renderer2D& renderer() { return renderer_; }

private:
  std::unique_ptr<AppBase> app_;
  bool didSetup_{false};

  int pixelW_{1};
  int pixelH_{1};
  float contentScale_{2.0f};  // Default Retina scale

  double elapsed_{0.0};
  double lastDt_{0.0};
  float frameRate_{60.0f};
  int frameNum_{0};

  DrawList2D drawList_;
  Renderer2D renderer_;
};

} // namespace oflike

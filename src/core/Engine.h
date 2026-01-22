#pragma once

#include <memory>
#include <set>

#include "AppBase.h"
#include "../render/DrawList.h"
#include "../render/metal/MetalRenderer.h"

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
  void tick(double dtSeconds, void* renderPassDescriptor, void* drawable) {
    if (!app_) return;

    lastDt_ = dtSeconds;
    elapsed_ += dtSeconds;
    frameNum_++;

    if (dtSeconds > 0) {
      float instantFps = 1.0f / static_cast<float>(dtSeconds);
      frameRate_ = frameRate_ * 0.9f + instantFps * 0.1f;
    }

    if (!didSetup_) {
      app_->setup();
      didSetup_ = true;
    }

    drawList_.reset();

    app_->update();

    renderer_.beginFrame(renderPassDescriptor, drawable, pixelW_, pixelH_, contentScale_);
    app_->draw();
    renderer_.draw(drawList_);
    renderer_.endFrame();
  }

  double elapsedTimeSeconds() const { return elapsed_; }
  double lastFrameTimeSeconds() const { return lastDt_; }

  int getWindowWidth() const {
    return contentScale_ > 0 ? static_cast<int>(pixelW_ / contentScale_) : pixelW_;
  }

  int getWindowHeight() const {
    return contentScale_ > 0 ? static_cast<int>(pixelH_ / contentScale_) : pixelH_;
  }

  int getPixelWidth() const { return pixelW_; }
  int getPixelHeight() const { return pixelH_; }

  float getFrameRate() const { return frameRate_; }
  int getFrameNum() const { return frameNum_; }

  // Mouse state
  void setMousePosition(int x, int y) { mouseX_ = x; mouseY_ = y; }
  void setMousePressed(int button, bool pressed) {
    if (button >= 0 && button < 3) mousePressed_[button] = pressed;
  }
  int getMouseX() const { return mouseX_; }
  int getMouseY() const { return mouseY_; }
  bool getMousePressed(int button = 0) const {
    return (button >= 0 && button < 3) ? mousePressed_[button] : false;
  }

  // Keyboard state
  void setKeyPressed(int key, bool pressed) {
    if (pressed) {
      keysPressed_.insert(key);
    } else {
      keysPressed_.erase(key);
    }
  }
  bool getKeyPressed(int key) const {
    return keysPressed_.find(key) != keysPressed_.end();
  }
  bool isAnyKeyPressed() const { return !keysPressed_.empty(); }

  // Modifier keys
  void setModifierKeys(bool shift, bool ctrl, bool alt, bool cmd) {
    shiftPressed_ = shift;
    ctrlPressed_ = ctrl;
    altPressed_ = alt;
    cmdPressed_ = cmd;
  }
  bool isShiftPressed() const { return shiftPressed_; }
  bool isCtrlPressed() const { return ctrlPressed_; }
  bool isAltPressed() const { return altPressed_; }
  bool isCmdPressed() const { return cmdPressed_; }

  DrawList& drawList() { return drawList_; }
  MetalRenderer& renderer() { return renderer_; }

private:
  std::unique_ptr<AppBase> app_;
  bool didSetup_{false};

  int pixelW_{1};
  int pixelH_{1};
  float contentScale_{2.0f};

  double elapsed_{0.0};
  double lastDt_{0.0};
  float frameRate_{60.0f};
  int frameNum_{0};

  int mouseX_{0};
  int mouseY_{0};
  bool mousePressed_[3]{false, false, false};

  std::set<int> keysPressed_;
  bool shiftPressed_{false};
  bool ctrlPressed_{false};
  bool altPressed_{false};
  bool cmdPressed_{false};

  DrawList drawList_;
  MetalRenderer renderer_;
};

} // namespace oflike

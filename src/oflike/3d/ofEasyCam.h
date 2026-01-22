#pragma once

#include "ofCamera.h"
#include "../math/ofVectorMath.h"

namespace oflike {

/// ofEasyCam - Mouse-controlled camera
/// Inherits from ofCamera, adds interactive mouse controls:
/// - Drag to rotate (orbit)
/// - Scroll to zoom
/// - Configurable sensitivity
class ofEasyCam : public ofCamera {
public:
  ofEasyCam();
  virtual ~ofEasyCam() = default;

  // Mouse input enable/disable
  void enableMouseInput();
  void disableMouseInput();
  bool isMouseInputEnabled() const { return bMouseInputEnabled_; }

  // Sensitivity settings
  void setDrag(float drag) { drag_ = drag; }
  float getDrag() const { return drag_; }
  void setRotationSensitivity(float x, float y, float z);
  void setRotationSensitivity(const glm::vec3& sensitivity);
  glm::vec3 getRotationSensitivity() const { return rotationSensitivity_; }
  void setTranslationSensitivity(float x, float y, float z);
  void setTranslationSensitivity(const glm::vec3& sensitivity);
  glm::vec3 getTranslationSensitivity() const { return translationSensitivity_; }

  // Mouse event handlers
  // These should be called from application's mouse event handlers
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void mouseScrolled(float x, float y);

protected:
  bool bMouseInputEnabled_ = true;
  float drag_ = 0.9f;           // Drag damping factor (0-1)
  glm::vec3 lastMouse_{0, 0, 0}; // Last mouse position
  bool bIsMousePressed_ = false; // Mouse button state
  int lastButton_ = -1;          // Last pressed button

  // Sensitivity factors
  glm::vec3 rotationSensitivity_{1.0f, 1.0f, 1.0f};
  glm::vec3 translationSensitivity_{1.0f, 1.0f, 1.0f};

  // Helper: Update camera rotation based on mouse delta
  void updateRotation(float deltaX, float deltaY);

  // Helper: Update camera distance (zoom) based on scroll
  void updateZoom(float scrollY);
};

} // namespace oflike

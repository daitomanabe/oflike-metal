#pragma once

#include <cassert>
#include "../core/Engine.hpp"

namespace oflike {

// Header-only global engine context.
// This keeps the user-facing API close to openFrameworks (global-style functions),
// while letting the platform layer own the concrete Engine instance.
inline Engine* gEngine = nullptr;

inline void setEngine(Engine* e) {
  gEngine = e;
}

inline Engine& engine() {
  assert(gEngine && "oflike::engine() called before Engine was set");
  return *gEngine;
}

} // namespace oflike

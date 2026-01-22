#pragma once

#include <cassert>
#include "Engine.h"

namespace oflike {

// Global engine context.
// This enables openFrameworks-style global function APIs.
inline Engine* gEngine = nullptr;

inline void setEngine(Engine* e) {
  gEngine = e;
}

inline Engine& engine() {
  assert(gEngine && "oflike::engine() called before Engine was set");
  return *gEngine;
}

} // namespace oflike

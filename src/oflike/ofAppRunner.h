#pragma once

#include <memory>
#include "../core/AppBase.hpp"

namespace oflike {

// The platform layer calls this factory to obtain the user app instance.
std::unique_ptr<AppBase> createApp();

} // namespace oflike

// Define the app factory function in a C++ translation unit.
// Usage:
//   class ofApp : public AppBase { ... };
//   OF_MAIN(ofApp)
#define OF_MAIN(APP_CLASS) \
  namespace oflike { \
  std::unique_ptr<AppBase> createApp() { \
    return std::make_unique<APP_CLASS>(); \
  } \
  }

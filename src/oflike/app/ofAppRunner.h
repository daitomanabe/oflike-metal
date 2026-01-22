#pragma once

#include <memory>
#include "../../core/AppBase.h"

// Forward declaration of createApp (defined by OF_MAIN macro in user code)
namespace oflike {
  std::unique_ptr<AppBase> createApp();
}

// Macro for creating main entry point
#define OF_MAIN(AppClass) \
  namespace oflike { \
    std::unique_ptr<AppBase> createApp() { \
      return std::make_unique<AppClass>(); \
    } \
  }

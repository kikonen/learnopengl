#pragma once

#include <string>
#include <vector>
#include <chrono>

#include <glm/glm.hpp>

#include "ki/size.h"

#include "util/Log.h"

#ifdef _DEBUG
  #define KI_GL_DEBUG_BREAK
#endif

#ifdef KI_GL_DEBUG_BREAK
    #define KI_BREAK() {Log::flush(); __debugbreak();}
#else
    #define KI_BREAK()
#endif

namespace ki {
}

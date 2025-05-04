#pragma once

// NOTE KI avoid crashes in release build due to incorrect fn args
// https://sol2.readthedocs.io/en/latest/safety.html
#define SOL_SAFE_FUNCTION_CALLS 1
#define SOL_ALL_SAFETIES_ON 1

#include <sol/sol.hpp>

//#include "binding/bind_vec3.h"

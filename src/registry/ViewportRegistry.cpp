#include "ViewportRegistry.h"

namespace {
    static ViewportRegistry s_registry;
}

ViewportRegistry& ViewportRegistry::get() noexcept
{
    return s_registry;
}

void ViewportRegistry::prepare()
{
}

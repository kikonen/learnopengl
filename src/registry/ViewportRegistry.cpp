#include "ViewportRegistry.h"

ViewportRegistry& ViewportRegistry::get() noexcept
{
    static ViewportRegistry s_registry;
    return s_registry;
}

void ViewportRegistry::prepare()
{
}

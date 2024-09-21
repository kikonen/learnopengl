#include "ViewportRegistry.h"

#include <algorithm>

namespace {
    static ViewportRegistry s_registry;
}

ViewportRegistry& ViewportRegistry::get() noexcept
{
    return s_registry;
}

void ViewportRegistry::addViewport(std::shared_ptr<Viewport> viewport) noexcept
{
    m_viewports.push_back(viewport);

    std::sort(m_viewports.begin(), m_viewports.end(), [](const auto& a, const auto& b) {
        return a->getOrder() < b->getOrder();
        });
}

void ViewportRegistry::prepare()
{
}

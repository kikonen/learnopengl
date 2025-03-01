#include "ViewportRegistry.h"

#include "util/thread.h"

#include <algorithm>

namespace {
    static ViewportRegistry s_registry;
}

ViewportRegistry& ViewportRegistry::get() noexcept
{
    return s_registry;
}

ViewportRegistry::ViewportRegistry() = default;
ViewportRegistry::~ViewportRegistry() = default;

void ViewportRegistry::clear()
{
    ASSERT_RT();

    m_viewports.clear();
}

void ViewportRegistry::shutdown()
{
    ASSERT_RT();

    clear();
}

void ViewportRegistry::prepare()
{
    ASSERT_RT();

    clear();
}

void ViewportRegistry::addViewport(std::shared_ptr<Viewport> viewport) noexcept
{
    ASSERT_RT();

    m_viewports.push_back(viewport);

    std::sort(m_viewports.begin(), m_viewports.end(), [](const auto& a, const auto& b) {
        return a->getOrder() < b->getOrder();
        });
}

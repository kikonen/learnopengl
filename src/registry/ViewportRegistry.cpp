#include "ViewportRegistry.h"

#include "util/thread.h"

#include <algorithm>

namespace
{
    static ViewportRegistry* s_registry{ nullptr };
}

void ViewportRegistry::init() noexcept
{
    assert(!s_registry);
    s_registry = new ViewportRegistry();
}

void ViewportRegistry::release() noexcept
{
    auto* s = s_registry;
    s_registry = nullptr;
    delete s;
}

ViewportRegistry& ViewportRegistry::get() noexcept
{
    assert(s_registry);
    return *s_registry;
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

void ViewportRegistry::addViewport(std::shared_ptr<model::Viewport> viewport) noexcept
{
    ASSERT_RT();

    m_viewports.push_back(viewport);

    std::sort(m_viewports.begin(), m_viewports.end(), [](const auto& a, const auto& b) {
        return a->getOrder() < b->getOrder();
        });
}

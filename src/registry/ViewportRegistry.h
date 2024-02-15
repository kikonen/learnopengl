#pragma once

#include <vector>

#include "model/Viewport.h"

class ViewportRegistry
{
public:
    static ViewportRegistry& get() noexcept;

    ViewportRegistry()
    {
    }
    ViewportRegistry& operator=(const ViewportRegistry&) = delete;

    void prepare();

    void addViewport(std::shared_ptr<Viewport> viewport) noexcept
    {
        m_viewports.push_back(viewport);
    }

    inline std::vector<std::shared_ptr<Viewport>>& getViewports() noexcept
    {
        return m_viewports;
    }

private:
    std::vector<std::shared_ptr<Viewport>> m_viewports;

};

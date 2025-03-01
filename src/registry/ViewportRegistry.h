#pragma once

#include <vector>

#include "model/Viewport.h"

class ViewportRegistry
{
public:
    static ViewportRegistry& get() noexcept;

    ViewportRegistry();
    ~ViewportRegistry();

    ViewportRegistry& operator=(const ViewportRegistry&) = delete;

    void clear();
    void shutdown();
    void prepare();

    void addViewport(std::shared_ptr<Viewport> viewport) noexcept;

    inline std::vector<std::shared_ptr<Viewport>>& getViewports() noexcept
    {
        return m_viewports;
    }

private:
    std::vector<std::shared_ptr<Viewport>> m_viewports;

};

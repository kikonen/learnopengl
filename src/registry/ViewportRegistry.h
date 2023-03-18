#pragma once

#include <vector>

#include "asset/Assets.h"

#include "model/Viewport.h"

class ViewportRegistry
{
public:
    ViewportRegistry(const Assets& assets)
        : m_assets(assets)
    {
    }

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
    const Assets& m_assets;

    std::vector<std::shared_ptr<Viewport>> m_viewports;

};

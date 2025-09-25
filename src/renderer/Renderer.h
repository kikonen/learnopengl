#pragma once

#include <string>

#include "pool/NodeHandle.h"

namespace model
{
    class Node;
}

namespace render
{
    class RenderContext;
}

class Program;

struct PrepareContext;
struct UpdateContext;
struct UpdateViewContext;

class Registry;

class Renderer
{
public:
    Renderer(
        std::string_view name,
        bool useFrameStep)
        : m_name(name),
        m_useFrameStep(useFrameStep)
    {}

    virtual ~Renderer();

    virtual void prepareRT(
        const PrepareContext& ctx);

    bool needRender(const render::RenderContext& ctx);

    inline bool isRendered() const noexcept {
        return m_rendered;
    }

    void setEnabled(bool enabled) {
        m_enabled = enabled;
    }

    inline bool isEnabled() const noexcept {
        return m_enabled;
    }

protected:
    // @return true if changed node
    bool setClosest(
        const render::RenderContext& ctx,
        model::Node* closest,
        int tagIndex);

    void clearClosest(
        const render::RenderContext& ctx,
        int tagIndex);

protected:
    bool m_prepared = false;

    std::string m_name;

    bool m_enabled{ false };

    Registry* m_registry{ nullptr };

    bool m_useFrameStep{ false };
    int m_renderFrameStart = 0;
    int m_renderFrameStep = 0;

    float m_elapsedSecs = -1;

    double m_lastHitTime{ 0.f };
    unsigned long m_lastHitFrame = 0;

    bool m_rendered = false;

    model::Node* m_lastClosest{ nullptr };
};

#pragma once

#include "asset/Assets.h"


class Program;
class Registry;

class UpdateContext;
class UpdateViewContext;
class RenderContext;

class Node;

class Renderer
{
public:
    Renderer(bool useFrameStep)
    : m_useFrameStep(useFrameStep)
    {}

    virtual ~Renderer();

    virtual void prepare(
        const Assets& assets,
        Registry* registry);

    bool needRender(const RenderContext& ctx);

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
    bool setClosest(Node* closest, int tagIndex);

protected:
    bool m_prepared = false;

    bool m_enabled{ false };

    Registry* m_registry{ nullptr };

    bool m_useFrameStep{ false };
    int m_renderFrameStart = 0;
    int m_renderFrameStep = 0;

    float m_elapsedSecs = -1;

    float m_lastHitTime = 0.f;
    unsigned long m_lastHitFrame = 0;

    bool m_rendered = false;

    Node* m_lastClosest{ nullptr };
};

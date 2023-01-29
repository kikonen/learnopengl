#pragma once

#include "asset/Assets.h"


class Shader;
class Registry;
class RenderContext;
class Node;

class Renderer
{
public:
    Renderer();
    virtual ~Renderer();

    virtual void prepare(
        const Assets& assets,
        Registry* registry);

    virtual void update(const RenderContext& ctx);

    bool needRender(const RenderContext& ctx);

protected:
    void setClosest(Node* closest, int tagIndex);

protected:
    bool m_prepared = false;

    Registry* m_registry{ nullptr };

    int m_renderFrameStart = 0;
    int m_renderFrameStep = 0;

    double m_elapsedSecs = -1;

    double m_lastHitTime = 0.f;
    unsigned long m_lastHitFrame = 0;

    bool m_rendered = false;

    Node* m_lastClosest{ nullptr };
};

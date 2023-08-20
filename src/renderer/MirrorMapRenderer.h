#pragma once

#include "Renderer.h"

#include "asset/Material.h"

#include "component/Camera.h"

class Node;
class FrameBuffer;
class Viewport;

class MirrorMapRenderer final : public Renderer
{
public:
    MirrorMapRenderer(bool useFrameStep) : Renderer(useFrameStep) {}
    ~MirrorMapRenderer() = default;

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    void bindTexture(const RenderContext& ctx);

    bool render(
        const RenderContext& ctx);

private:
    void drawNodes(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer,
        Node* current);

    Node* findClosest(const RenderContext& ctx);

public:
    std::shared_ptr<Viewport> m_debugViewport;

private:
    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;

    std::vector<Camera> m_cameras;

    std::unique_ptr<FrameBuffer> m_prev{ nullptr };
    std::unique_ptr<FrameBuffer> m_curr{ nullptr };

    Material m_tagMaterial;
};

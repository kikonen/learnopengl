#pragma once

#include "Renderer.h"

#include "asset/Material.h"

#include "component/Camera.h"

class Node;
class FrameBuffer;
class Viewport;
class WaterMapRenderer;

class MirrorMapRenderer final : public Renderer
{
public:
    MirrorMapRenderer(
        bool useFrameStep,
        bool doubleBuffer,
        bool squareAspectRatio)
        : Renderer(useFrameStep),
        m_doubleBuffer(doubleBuffer),
        m_squareAspectRatio(squareAspectRatio) {}

    ~MirrorMapRenderer() = default;

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    void updateView(const RenderContext& ctx);

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
    std::shared_ptr<Viewport> m_reflectionDebugViewport;

    Node* m_sourceNode{ nullptr };

private:
    const bool m_doubleBuffer;
    const bool m_squareAspectRatio;

    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;

    std::vector<Camera> m_cameras;

    int m_width{ -1 };
    int m_height{ -1 };

    int m_currIndex{ 0 };
    int m_prevIndex{ 0 };

    int m_bufferCount{ 1 };
    std::vector<std::unique_ptr<FrameBuffer>> m_reflectionBuffers;

    std::unique_ptr<WaterMapRenderer> m_waterMapRenderer{ nullptr };
    std::unique_ptr<MirrorMapRenderer> m_mirrorMapRenderer{ nullptr };

    Material m_tagMaterial;
};

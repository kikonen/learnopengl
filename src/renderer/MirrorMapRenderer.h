#pragma once

#include "Renderer.h"

#include "asset/Material.h"

#include "component/Camera.h"

namespace render {
    class FrameBuffer;
}

class Node;
class Viewport;
class WaterMapRenderer;

class EditorFrame;

class MirrorMapRenderer final : public Renderer
{
    friend class EditorFrame;

public:
    MirrorMapRenderer(
        std::string_view name,
        bool useFrameStep,
        bool doubleBuffer,
        bool squareAspectRatio)
        : Renderer(name, useFrameStep),
        m_doubleBuffer(doubleBuffer),
        m_squareAspectRatio(squareAspectRatio) {}

    ~MirrorMapRenderer() = default;

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void updateRT(const UpdateViewContext& ctx);

    void bindTexture(const RenderContext& ctx);

    bool render(
        const RenderContext& ctx);

    void handleNodeAdded(Node* node);

private:
    void drawNodes(
        const RenderContext& ctx,
        render::FrameBuffer* targetBuffer,
        Node* current);

    Node* findClosest(const RenderContext& ctx);

public:
    std::shared_ptr<Viewport> m_reflectionDebugViewport;

    pool::NodeHandle m_sourceNode{};

private:
    const bool m_doubleBuffer;
    const bool m_squareAspectRatio;

    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;

    std::vector<Camera> m_cameras;

    int m_reflectionWidth{ -1 };
    int m_reflectionheight{ -1 };

    int m_currIndex{ 0 };
    int m_prevIndex{ 0 };

    int m_bufferCount{ 1 };
    std::vector<std::unique_ptr<render::FrameBuffer>> m_reflectionBuffers;

    std::unique_ptr<WaterMapRenderer> m_waterMapRenderer{ nullptr };
    std::unique_ptr<MirrorMapRenderer> m_mirrorMapRenderer{ nullptr };

    Material m_tagMaterial;

    std::vector<pool::NodeHandle> m_nodes;
};

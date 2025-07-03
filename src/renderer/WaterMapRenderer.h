#pragma once

#include <vector>

#include "Renderer.h"

#include "material/Material.h"

#include "render/Camera.h"

namespace kigl {
    class GLState;
}

namespace render {
    class FrameBuffer;
    class NodeDraw;
}

namespace editor {
    class EditorFrame;
    class ViewportTool;
}

class Node;
class Viewport;

class WaterMapRenderer final : public Renderer
{
    friend class editor::EditorFrame;
    friend class editor::ViewportTool;

public:
    WaterMapRenderer(
        std::string_view name,
        bool useFrameStep,
        bool doubleBuffer,
        bool squareAspectRatio)
        : Renderer(name, useFrameStep),
        m_doubleBuffer(doubleBuffer),
        m_squareAspectRatio(squareAspectRatio) {}

    virtual ~WaterMapRenderer() override;

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void updateRT(const UpdateViewContext& ctx);

    void bindTexture(kigl::GLState& state);

    bool render(
        const RenderContext& ctx);

private:
    void updateReflectionView(const UpdateViewContext& ctx);
    void updateRefractionView(const UpdateViewContext& ctx);

    void drawNodes(
        const RenderContext& ctx,
        render::FrameBuffer* targetBuffer,
        Node* current,
        bool reflect);

    Node* findClosest(const RenderContext& ctx);

public:
    std::shared_ptr<Viewport> m_reflectionDebugViewport;
    std::shared_ptr<Viewport> m_refractionDebugViewport;

    pool::NodeHandle m_sourceNode{};

private:
    const bool m_doubleBuffer;
    const bool m_squareAspectRatio;

    float m_nearPlane{ 0.1f };
    float m_farPlane{ 1000.0f };


    std::vector<render::Camera> m_cameras;

    int m_width{ -1 };
    int m_height{ -1 };

    int m_reflectionWidth{ -1 };
    int m_reflectionHeight{ -1 };

    int m_refractionWidth{ -1 };
    int m_refractionHeight{ -1 };

    int m_currIndex{ 0 };
    int m_prevIndex{ 0 };

    int m_bufferCount{ 1 };
    std::vector<std::unique_ptr<render::FrameBuffer>> m_reflectionBuffers;
    std::vector<std::unique_ptr<render::FrameBuffer>> m_refractionBuffers;

    std::unique_ptr<render::NodeDraw> m_nodeDraw;

    unsigned int m_noiseTextureID{ 0 };

    Material m_tagMaterial;
};

#pragma once

#include <vector>

#include "Renderer.h"

#include "material/Material.h"

#include "render/Camera.h"

namespace kigl {
    class GLState;
}

namespace render {
    class CubeMapBuffer;
    class FrameBuffer;
    class NodeDraw;
}

namespace model
{
    class Node;
}

class DynamicCubeMap;
class WaterMapRenderer;
class MirrorMapRenderer;

namespace editor {
    class EditorFrame;
    class ViewportTool;
}

class CubeMapRenderer final : public Renderer
{
    friend class editor::EditorFrame;
    friend class editor::ViewportTool;

public:
    CubeMapRenderer(bool useFrameStep)
        : Renderer("main", useFrameStep) {}

    virtual ~CubeMapRenderer() override;

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void updateRT(const UpdateViewContext& ctx);

    void bindTexture(kigl::GLState& state);

    bool render(
        const render::RenderContext& ctx);

private:
    void clearCubeMap(
        const render::RenderContext& ctx,
        DynamicCubeMap& cube);

    void drawNodes(
        const render::RenderContext& ctx,
        render::CubeMapBuffer* targetBuffer,
        const model::Node* centerNode,
        const glm::vec4& debugColor);

    model::Node* findClosest(
        const render::RenderContext& ctx);

public:

private:
    //glm::vec3 center{ 0, 0, 0 };

    float m_nearPlane{ 0.1f };
    float m_farPlane{ 500.0f };

    bool m_cleared{ false };

    std::unique_ptr<DynamicCubeMap> m_prev;
    std::unique_ptr<DynamicCubeMap> m_curr;

    std::vector<render::Camera> m_cameras;

    Material m_tagMaterial;

    std::unique_ptr<WaterMapRenderer> m_waterMapRenderer{ nullptr };
    std::unique_ptr<MirrorMapRenderer> m_mirrorMapRenderer{ nullptr };

    std::unique_ptr<render::NodeDraw> m_nodeDraw;
};

#pragma once

#include "Renderer.h"

#include "asset/Material.h"

#include "component/Camera.h"

class FrameBuffer;
class DynamicCubeMap;
class WaterMapRenderer;

class CubeMapRenderer final : public Renderer
{
public:
    CubeMapRenderer(bool useFrameStep) : Renderer(useFrameStep) {}
    ~CubeMapRenderer();

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    void updateView(const RenderContext& ctx);

    void bindTexture(const RenderContext& ctx);

    bool render(
        const RenderContext& ctx);

private:
    void clearCubeMap(
        const RenderContext& ctx,
        DynamicCubeMap& cube);

    void drawNodes(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer,
        const Node* centerNode,
        const glm::vec4& debugColor);

    Node* findCenter(
        const RenderContext& ctx);

    Node* getTagNode();

public:

private:
    //glm::vec3 center{ 0, 0, 0 };

    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;

    bool m_cleared{ false };

    std::unique_ptr<DynamicCubeMap> m_prev;
    std::unique_ptr<DynamicCubeMap> m_curr;

    std::vector<Camera> m_cameras;

    Material m_tagMaterial;

    uuids::uuid m_tagID;
    Node* m_tagNode{ nullptr };

    std::unique_ptr<WaterMapRenderer> m_waterMapRenderer{ nullptr };
};

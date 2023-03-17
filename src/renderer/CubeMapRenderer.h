#pragma once

#include "Renderer.h"

#include "asset/Material.h"
#include "asset/DynamicCubeMap.h"

#include "component/Camera.h"

#include "render/FrameBuffer.h"


class CubeMapRenderer final : public Renderer
{
public:
    CubeMapRenderer() {}
    virtual ~CubeMapRenderer() = default;

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    void bindTexture(const RenderContext& ctx);

    bool render(
        const RenderContext& ctx);

private:
    void clearCubeMap(
        const RenderContext& ctx,
        DynamicCubeMap& cube,
        const glm::vec4& color,
        bool debug);

    void drawNodes(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer,
        const Node* centerNode,
        const glm::vec4& clearColor);

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
};

#pragma once

#include "Renderer.h"

#include "asset/Material.h"

#include "component/Camera.h"

class Node;
class FrameBuffer;
class Viewport;
class FrameBuffer;

class WaterMapRenderer final : public Renderer
{
public:
    WaterMapRenderer() {}
    virtual ~WaterMapRenderer() = default;

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
        Node* current,
        bool reflect);

    Node* findClosest(const RenderContext& ctx);

public:
    std::shared_ptr<Viewport> m_reflectionDebugViewport;
    std::shared_ptr<Viewport> m_refractionDebugViewport;

private:
    float m_nearPlane{ 0.1f };
    float m_farPlane{ 1000.0f };

    std::vector<Camera> m_cameras;

    std::unique_ptr<FrameBuffer> m_reflectionBuffer{ nullptr };
    std::unique_ptr<FrameBuffer> m_refractionBuffer{ nullptr };

    unsigned int m_noiseTextureID{ 0 };

    Material m_tagMaterial;
};

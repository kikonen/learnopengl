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
    WaterMapRenderer(
        bool useFrameStep,
        bool doubleBuffer,
        bool squareAspectRatio)
        : Renderer(useFrameStep),
        m_doubleBuffer(doubleBuffer),
        m_squareAspectRatio(squareAspectRatio) {}

    virtual ~WaterMapRenderer() = default;

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    void updateView(const UpdateViewContext& ctx);

    void bindTexture(const RenderContext& ctx);

    bool render(
        const RenderContext& ctx);

private:
    void updateReflectionView(const UpdateViewContext& ctx);
    void updateRefractionView(const UpdateViewContext& ctx);

    void drawNodes(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer,
        Node* current,
        bool reflect);

    Node* findClosest(const RenderContext& ctx);

public:
    std::shared_ptr<Viewport> m_reflectionDebugViewport;
    std::shared_ptr<Viewport> m_refractionDebugViewport;

    Node* m_sourceNode{ nullptr };

private:
    const bool m_doubleBuffer;
    const bool m_squareAspectRatio;

    float m_nearPlane{ 0.1f };
    float m_farPlane{ 1000.0f };


    std::vector<Camera> m_cameras;

    int m_reflectionWidth{ -1 };
    int m_reflectionheight{ -1 };

    int m_refractionWidth{ -1 };
    int m_refractionHeight{ -1 };

    int m_currIndex{ 0 };
    int m_prevIndex{ 0 };

    int m_bufferCount{ 1 };
    std::vector<std::unique_ptr<FrameBuffer>> m_reflectionBuffers;
    std::vector<std::unique_ptr<FrameBuffer>> m_refractionBuffers;

    unsigned int m_noiseTextureID{ 0 };

    Material m_tagMaterial;
};

#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "Renderer.h"

#include "pool/NodeHandle.h"

#include "render/Camera.h"
#include "render/IrradianceMap.h"
#include "render/PrefilterMap.h"

namespace kigl {
    class GLState;
}

namespace render {
    class RenderContext;
    class CubeMapBuffer;
    class NodeDraw;
}

namespace model
{
    class Node;
}

class DynamicCubeMap;

struct PrepareContext;
struct UpdateViewContext;

// Bakes scene-captured Image-Based Lighting from environment_probe nodes.
//
// Captures the scene from a probe node into a radiance cube, generates its mip chain, then
// convolves it into an irradiance + prefilter map so deferred IBL reflects actual scenery
// instead of just the skybox.
//
// Phase 1: a single global probe (closest to camera) feeding the existing UNIT_IRRADIANCE_MAP /
// UNIT_PREFILTER_MAP, overriding the skybox-derived maps for the frame. Falls back to skybox IBL
// when no bake has happened yet (or when disabled).
//
// NOTE: distinct from EnvironmentProbeRenderer (a debug wireframe visualizer of probe positions).
class IblProbeRenderer final : public Renderer
{
public:
    IblProbeRenderer(bool useFrameStep)
        : Renderer("ibl_probe", useFrameStep) {}

    virtual ~IblProbeRenderer() override;

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void updateRT(const UpdateViewContext& ctx);

    // Bind the convolved IBL maps over units 71/72. No-op until the first bake (so the
    // skybox-derived maps bound earlier this frame remain the fallback).
    void bindTexture(kigl::GLState& state);

    bool render(
        const render::RenderContext& ctx);

    // Per-frame probe metadata (world pos + influence bounds). Built by enumerateProbes;
    // consumed by the per-fragment probe blend (Phase 4). Phase 2 still bakes only the closest.
    struct ProbeMeta {
        pool::NodeHandle node;
        glm::vec3 pos{ 0.f };
        float innerRadius{ 0.f };
        float outerRadius{ 0.f };   // <= 0 => global (covers everything)
        int index{ 0 };
    };

    const std::vector<ProbeMeta>& getProbes() const noexcept { return m_probes; }

private:
    // Collect environment_probe nodes into m_probes (pos + radii from node type).
    void enumerateProbes(const render::RenderContext& ctx);

    void drawNodes(
        const render::RenderContext& ctx,
        render::CubeMapBuffer* targetBuffer,
        const model::Node* centerNode);

    model::Node* findClosest(
        const render::RenderContext& ctx);

private:
    float m_nearPlane{ 0.1f };
    float m_farPlane{ 500.0f };

    int m_envSize{ 0 };

    bool m_baked{ false };

    std::unique_ptr<DynamicCubeMap> m_captureCube;

    std::vector<render::Camera> m_cameras;

    render::IrradianceMap m_irradianceMap;
    render::PrefilterMap m_prefilterMap;

    std::vector<ProbeMeta> m_probes;

    std::unique_ptr<render::NodeDraw> m_nodeDraw;
};

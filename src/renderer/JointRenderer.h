#pragma once

#include <memory>

#include "MeshRenderer.h"

namespace mesh {
    class Mesh;
}

class JointRenderer : public MeshRenderer
{
public:
    JointRenderer();
    ~JointRenderer();

    virtual void prepareRT(const PrepareContext& ctx) override;

    virtual void render(
        const render::RenderContext& ctx,
        render::FrameBuffer* targetBuffer) override;

private:
    std::shared_ptr<mesh::Mesh> m_mesh;
    ki::material_index m_materialIndex{ 0 };
};

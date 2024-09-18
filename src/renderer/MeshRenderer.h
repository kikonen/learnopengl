#pragma once

#include "material/Material.h"

class Program;
class RenderContext;
struct PrepareContext;

namespace mesh {
    struct MeshInstance;
    class Mesh;
}

namespace render {
    class FrameBuffer;
}

class MeshRenderer
{
public:
    virtual void prepareRT(const PrepareContext& ctx);

    virtual void render(
        const RenderContext& ctx,
        render::FrameBuffer* targetBuffer) = 0;

protected:
    void drawObjects(
        const RenderContext& ctx,
        render::FrameBuffer* targetBuffer,
        const std::vector<mesh::MeshInstance>& meshes);

protected:
    Material m_fallbackMaterial;

    ki::program_id m_programId;

    uint32_t m_entityIndex{ 0 };
};

#pragma once

#include <vector>

#include "material/Material.h"

namespace render
{
    class RenderContext;
}

class Program;
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
    MeshRenderer();
    ~MeshRenderer();

    virtual void prepareRT(const PrepareContext& ctx);

    virtual void render(
        const render::RenderContext& ctx,
        render::FrameBuffer* targetBuffer) = 0;

protected:
    void drawObjects(
        const render::RenderContext& ctx,
        render::FrameBuffer* targetBuffer,
        const std::vector<mesh::MeshInstance>& meshes);

protected:
    Material m_fallbackMaterial;

    ki::program_id m_programId;

private:
    uint32_t m_entityIndex{ 0 };

    int m_dynamicVaoIndex{ -1 };

    bool m_useFenceDebug{ false };
};

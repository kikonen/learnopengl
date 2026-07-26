#pragma once

#include <vector>

#include "material/Material.h"

#include "util/BufferReference.h"


namespace render
{
    class InstanceRegistry;
    class RenderContext;
}

class Program;
struct PrepareContext;
struct UpdateContext;

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

    void update(
        const UpdateContext& ctx);

    void beginFrame();
    void endFrame();

    void draw(
        const render::RenderContext& ctx,
        render::FrameBuffer* targetBuffer);

protected:
    virtual void updateImpl(
        const UpdateContext& ctx) = 0;

    void updateMeshes(
        const UpdateContext& ctx,
        const std::vector<mesh::MeshInstance>& meshes);

    void registerDrawables(
        const std::vector<mesh::MeshInstance>& meshes,
        render::InstanceRegistry& instanceRegistry) noexcept;

protected:
    util::Ref<Material> m_fallbackMaterial;

    ki::program_id m_programId;

private:
    uint32_t m_entityIndex{ 0 };

    size_t m_currentDrawableCount{ 0 };
    util::BufferReference m_instanceRef;
};

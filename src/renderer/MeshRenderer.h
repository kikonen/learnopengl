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
struct UpdateViewContext;

namespace mesh {
    struct MeshInstance;
    class Mesh;
    class TexturedVAO;
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
        const UpdateViewContext& ctx);

    void endFrame();

    void draw(
        const render::RenderContext& ctx,
        render::FrameBuffer* targetBuffer);

protected:
    virtual void updateImpl(
        const UpdateViewContext& ctx) = 0;

    void updateMeshes(
        const UpdateViewContext& ctx,
        const std::vector<mesh::MeshInstance>& meshes);

    void registerDrawables(
        const std::vector<mesh::MeshInstance>& meshes,
        render::InstanceRegistry& instanceRegistry) noexcept;

protected:
    util::Ref<Material> m_fallbackMaterial;

    ki::program_id m_programId;

private:
    uint32_t m_entityIndex{ 0 };

    int m_dynamicVaoIndex{ -1 };

    bool m_useFenceDebug{ false };

    mesh::TexturedVAO* m_currentDynamicVao = nullptr;
    size_t m_currentDrawableCount{ 0 };

    util::BufferReference m_instanceRef;
};

#pragma once

#include <span>
#include <functional>
#include <mutex>

#include "asset/SphereVolume.h"

#include "model/NodeState.h"

#include "pool/NodeHandle.h"

#include "kigl/kigl.h"

#include "util/BufferReference.h"

#include "GeneratorMode.h"

namespace kigl {
    struct GLVertexArray;
}

namespace backend {
    struct DrawOptions;
    struct Lod;
}

namespace render {
    class Batch;
    class InstanceRegistry;
    struct DrawableInfo;
}

namespace mesh {
    struct LodMesh;
    struct Transform;
}

namespace model
{
    class Node;
    struct NodeState;
    struct Snapshot;
}

namespace render
{
    class RenderContext;
}

struct EntitySSBO;

class Program;

struct PrepareContext;
struct UpdateContext;

class EntityRegistry;


//
// Generate node OR entity instances for node
//
class NodeGenerator
{
public:
    NodeGenerator() = default;
    virtual ~NodeGenerator();

    inline bool isLightWeight() const noexcept
    {
        return m_lightWeight;
    }

    inline bool isLightWeightPhysics() const noexcept
    {
        return m_lightWeightPhysics;
    }

    virtual void prepareWT(
        const PrepareContext& ctx,
        model::Node& container) {}

    virtual void prepareRT(
        const PrepareContext& ctx,
        model::Node& container,
        const model::Snapshot& snapshot) {}

    virtual void updateWT(
        const UpdateContext& ctx,
        const model::Node& container) {}

    virtual void registerDrawables(
        render::InstanceRegistry& instanceRegistry,
        const model::Node& container,
        const model::Snapshot& snapshot);

    virtual void updateDrawables(
        render::InstanceRegistry& instanceRegistry,
        const model::Node& container,
        const model::Snapshot& snapshot);

    virtual void bindBatch(
        const render::RenderContext& ctx,
        const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
        const std::function<void(ki::program_id)>& programPrepare,
        uint8_t kindBits,
        render::Batch& batch,
        const model::Node& container);

    virtual void updateVAO(
        const render::RenderContext& ctx,
        const model::Node& container) {}

    virtual const std::vector<mesh::LodMesh>* getLodMeshes(const model::Node& container) const
    {
        return nullptr;
    }

    const std::vector<mesh::Transform>& getTransforms() const noexcept
    {
        return m_transforms;
    }

    void markDirty(util::BufferReference ref);

public:
    GeneratorMode m_mode{ GeneratorMode::none };

    glm::vec3 m_offset{ 0.f };
    float m_scale{ 1.f };

protected:
    bool m_lightWeight{ false };
    bool m_lightWeightPhysics{ false };

    uint32_t m_poolSize{ 0 };

    ki::level_id m_containerMatrixLevel{ 0 };

    SphereVolume m_localVolume{ 0.f };

    util::BufferReference m_instanceRef;

    std::vector<mesh::Transform> m_transforms;
    std::vector<uint32_t> m_transformIndeces;

    std::vector<util::BufferReference> m_dirtySlots;

    std::vector<pool::NodeHandle> m_nodes;
};

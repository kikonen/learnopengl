#pragma once

#include <span>
#include <functional>

#include "model/NodeState.h"

#include "pool/NodeHandle.h"

#include "kigl/kigl.h"

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
}

namespace mesh {
    struct LodMesh;
    struct MeshTransform;
}

class Program;
class Node;

struct Snapshot;
struct EntitySSBO;

struct PrepareContext;
struct UpdateContext;
class RenderContext;

class EntityRegistry;

struct NodeState;

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
        Node& container) {}

    virtual void prepareRT(
        const PrepareContext& ctx,
        Node& container) {}

    virtual void updateWT(
        const UpdateContext& ctx,
        const Node& container) {}

    virtual void bindBatch(
        const RenderContext& ctx,
        const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
        const std::function<void(ki::program_id)>& programPrepare,
        uint8_t kindBits,
        render::Batch& batch,
        const Node& container,
        const Snapshot& snapshot);

    virtual void updateVAO(
        const RenderContext& ctx,
        const Node& container) {}

    virtual const std::vector<mesh::LodMesh>* getLodMeshes(const Node& container) const
    {
        return nullptr;
    }

public:
    GeneratorMode m_mode{ GeneratorMode::none };

    glm::vec3 m_offset{ 0.f };
    float m_scale{ 1.f };

protected:
    bool m_lightWeight{ false };
    bool m_lightWeightPhysics{ false };

    uint32_t m_poolSize{ 0 };

    ki::level_id m_containerMatrixLevel{ 0 };

    glm::vec4 m_volume{ 0.f };

    std::vector<uint32_t> m_indeces;
    std::vector<mesh::MeshTransform> m_transforms;
    std::vector<pool::NodeHandle> m_nodes;
};

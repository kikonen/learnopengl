#pragma once

#include <span>
#include <functional>

#include "model/NodeState.h"
#include "model/InstancePhysics.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "kigl/kigl.h"

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
}

class Program;
class Node;

struct Snapshot;
struct EntitySSBO;

struct PrepareContext;
struct UpdateContext;
class RenderContext;

class NodeSnapshotRegistry;
class EntityRegistry;

//
// Generate node OR entity instances for node
//
class NodeGenerator
{
public:
    NodeGenerator() = default;
    virtual ~NodeGenerator();

    virtual void prepare(
        const PrepareContext& ctx,
        Node& container) {}

    virtual void prepareRT(
        const PrepareContext& ctx,
        Node& container) {}

    virtual void updateWT(
        const UpdateContext& ctx,
        Node& container) {}

    void snapshotWT(
        NodeSnapshotRegistry& snapshotRegistry);

    virtual void updateEntity(
        NodeSnapshotRegistry& snapshotRegistry,
        EntityRegistry& entityRegistry,
        Node& container);

    virtual void bindBatch(
        const RenderContext& ctx,
        mesh::MeshType* type,
        const std::function<Program* (const mesh::LodMesh&)>& programSelector,
        uint8_t kindBits,
        render::Batch& batch,
        Node& container);

    virtual void updateVAO(
        const RenderContext& ctx,
        const Node& container) {}

    inline const std::vector<NodeState>& getStates() noexcept
    {
        return m_states;
    }

    inline std::vector<NodeState>& modifyStates() noexcept
    {
        return m_states;
    }

    std::span<const Snapshot> getSnapshots(
        NodeSnapshotRegistry& snapshotRegistry) const noexcept;

protected:
    void prepareSnapshots(
        NodeSnapshotRegistry& snapshotRegistry);

    void prepareEntities(
        EntityRegistry& entityRegistry);

    virtual void prepareEntity(
        EntitySSBO& entity,
        uint32_t index) {}

    inline void setActiveRange(uint32_t index, uint32_t count) noexcept {
        m_activeFirst  = index;
        m_activeCount = count;
    }

    glm::vec4 calculateVolume() const noexcept;

protected:
    uint32_t m_poolSize = 0;

    uint32_t m_activeFirst = 0;
    uint32_t m_activeCount = 0;

    uint32_t m_snapshotBase{ 0 };
    uint32_t m_entityBase{ 0 };

    uint32_t m_reservedCount{ 0 };

    ki::level_id m_containerMatrixLevel{ 0 };

    std::vector<NodeState> m_states;
    std::vector<const backend::Lod*> m_lods;
    std::vector<InstancePhysics> m_physics;
};

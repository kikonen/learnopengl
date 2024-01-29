#pragma once

#include "model/NodeTransform.h"
#include "model/InstancePhysics.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "kigl/kigl.h"

namespace kigl {
    struct GLVertexArray;
}

namespace backend {
    struct DrawOptions;
}

namespace render {
    class Batch;
}

class Node;

class Assets;
struct Snapshot;
struct EntitySSBO;

struct PrepareContext;
struct UpdateContext;
class RenderContext;

class SnapshotRegistry;
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
        SnapshotRegistry& snapshotRegistry);

    virtual void updateEntity(
        const Assets& assets,
        SnapshotRegistry& snapshotRegistry,
        EntityRegistry& entityRegistry,
        Node& container);

    virtual void bindBatch(
        const RenderContext& ctx,
        Node& container,
        render::Batch& batch);

    virtual void updateVAO(
        const RenderContext& ctx,
        const Node& container) {}

    virtual const kigl::GLVertexArray* getVAO(
        const Node& container) const noexcept;

    virtual const backend::DrawOptions& getDrawOptions(
        const Node& container) const noexcept;

    inline const std::vector<NodeTransform>& getTransforms() noexcept
    {
        return m_transforms;
    }

    inline std::vector<NodeTransform>& modifyTransforms() noexcept
    {
        return m_transforms;
    }

protected:
    void prepareSnapshots(
        SnapshotRegistry& snapshotRegistry);

    void prepareEntities(
        EntityRegistry& entityRegistry);

    virtual void prepareEntity(
        EntitySSBO& entity,
        uint32_t index) {}

    inline void setActiveRange(uint32_t index, uint32_t count) noexcept {
        m_activeFirst  = index;
        m_activeCount = count;
    }

    const glm::vec4 calculateVolume();

protected:
    uint32_t m_poolSize = 0;

    uint32_t m_activeFirst = 0;
    uint32_t m_activeCount = 0;

    uint32_t m_snapshotBase{ 0 };
    uint32_t m_entityBase{ 0 };

    uint32_t m_reservedCount{ 0 };

    int m_containerMatrixLevel = -1;

    std::vector<NodeTransform> m_transforms;
    std::vector<InstancePhysics> m_physics;
};

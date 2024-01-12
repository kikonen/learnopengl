#pragma once

#include "model/NodeTransform.h"
#include "model/Snapshot.h"
#include "model/InstancePhysics.h"

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

struct PrepareContext;
struct UpdateContext;
class RenderContext;

class EntityRegistry;

//
// Generate node OR entity instances for node
//
class NodeGenerator
{
public:
    NodeGenerator() = default;
    virtual ~NodeGenerator() = default;

    virtual void prepare(
        const PrepareContext& ctx,
        Node& container) {}

    virtual void update(
        const UpdateContext& ctx,
        Node& container) {}

    void snapshotWT(bool force);
    void snapshotRT(bool force);

    virtual void updateEntity(
        const UpdateContext& ctx,
        Node& container,
        EntityRegistry* entityRegistry,
        bool force);

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
    void prepareEntities(
        const PrepareContext& ctx);

    virtual void prepareEntity(
        const PrepareContext& ctx,
        Snapshot& snapshot,
        uint32_t snapshotIndex) {}

    //inline const std::vector<Snapshot>& getSnapshots() noexcept
    //{
    //    return m_snapshots;
    //}

    //inline std::vector<Snapshot>& modifySnapshots() noexcept
    //{
    //    return m_snapshots;
    //}

    //inline int getActiveFirst() const noexcept {
    //    return m_activeFirst;
    //}

    //inline int getActiveCount() const noexcept {
    //    return m_activeCount;
    //}

    //inline int getReservedFirst() const noexcept {
    //    return m_reservedFirst;
    //}

    //inline int getReservedCount() const noexcept {
    //    return m_reservedCount;
    //}

    inline void setActiveRange(uint32_t index, uint32_t count) noexcept {
        m_activeFirst  = index;
        m_activeCount = count;
    }

    const glm::vec4 calculateVolume();

protected:
    uint32_t m_poolSize = 0;

    uint32_t m_activeFirst = 0;
    uint32_t m_activeCount = 0;

    int m_reservedFirst = -1;
    uint32_t m_reservedCount{ 0 };

    int m_containerMatrixLevel = -1;

    std::vector<NodeTransform> m_transforms;
    std::vector<Snapshot> m_snapshotsWT;
    std::vector<Snapshot> m_snapshotsRT;
    std::vector<InstancePhysics> m_physics;
};

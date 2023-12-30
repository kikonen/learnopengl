#pragma once

#include "asset/Assets.h"

#include "model/NodeTransform.h"
#include "model/Snapshot.h"
#include "model/InstancePhysics.h"

#include "kigl/kigl.h"

class Node;
class Registry;

class UpdateContext;
class RenderContext;

class Batch;
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
        const Assets& assets,
        Registry* registry,
        Node& container) {}

    virtual void update(
        const UpdateContext& ctx,
        Node& container) {}

    void snapshot(bool force);

    virtual void updateEntity(
        const UpdateContext& ctx,
        Node& container,
        EntityRegistry* entityRegistry,
        bool force);

    void bindBatch(
        const RenderContext& ctx,
        Node& container,
        Batch& batch);

    inline const std::vector<NodeTransform>& getTransforms() noexcept
    {
        return m_transforms;
    }

    inline std::vector<NodeTransform>& modifyTransforms() noexcept
    {
        return m_transforms;
    }

    //inline const std::vector<Snapshot>& getSnapshots() noexcept
    //{
    //    return m_snapshots;
    //}

    //inline std::vector<Snapshot>& modifySnapshots() noexcept
    //{
    //    return m_snapshots;
    //}

    inline int getActiveFirst() const noexcept {
        return m_activeFirst;
    }

    inline int getActiveCount() const noexcept {
        return m_activeCount;
    }

    inline int getReservedFirst() const noexcept {
        return m_reservedFirst;
    }

    inline int getReservedCount() const noexcept {
        return m_reservedCount;
    }

    inline void setActiveRange(int index, int count) noexcept {
        m_activeFirst  = index;
        m_activeCount = count;
    }

    const glm::vec4 calculateVolume();

protected:
    size_t m_poolSize = 0;

    int m_activeFirst = 0;
    int m_activeCount = 0;

    int m_reservedFirst = -1;
    int m_reservedCount = 0;

    int m_containerMatrixLevel = -1;

    std::vector<NodeTransform> m_transforms;
    std::vector<Snapshot> m_snapshots;
    std::vector<InstancePhysics> m_physics;
};

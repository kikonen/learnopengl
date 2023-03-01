#pragma once

#include "asset/Assets.h"

#include "model/NodeInstance.h"
#include "model/InstancePhysics.h"

#include "ki/GL.h"

class Node;
class Registry;
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
        const RenderContext& ctx,
        Node& container,
        Node* containerParent) {}

    virtual void updateEntity(
        const RenderContext& ctx,
        Node& container,
        EntityRegistry* entityRegistry);

    void bindBatch(
        const RenderContext& ctx,
        Node& container,
        Batch& batch);

    inline std::vector<NodeInstance>& getInstances() noexcept
    {
        return m_instances;
    }

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

    const glm::vec4& calculateVolume();

protected:
    size_t m_poolSize = 0;

    int m_activeFirst = 0;
    int m_activeCount = 0;

    int m_reservedFirst = -1;
    int m_reservedCount = 0;

    int m_containerMatrixLevel = -1;

    std::vector<NodeInstance> m_instances;
    std::vector<InstancePhysics> m_physics;
};

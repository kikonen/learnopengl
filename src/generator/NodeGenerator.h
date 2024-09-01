#pragma once

#include <span>
#include <functional>

#include "model/NodeState.h"
#include "model/InstancePhysics.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

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

struct NodeState;

//
// Generate node OR entity instances for node
//
class NodeGenerator
{
public:
    NodeGenerator() = default;
    virtual ~NodeGenerator();

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
        mesh::MeshType* type,
        const std::function<Program* (const mesh::LodMesh&)>& programSelector,
        uint8_t kindBits,
        render::Batch& batch,
        const Node& container) {}

    virtual void updateVAO(
        const RenderContext& ctx,
        const Node& container) {}

public:
    GeneratorMode m_mode{ GeneratorMode::none };

protected:
    bool m_setupDone{ false };

    uint32_t m_poolSize = 0;

    ki::level_id m_containerMatrixLevel{ 0 };

    std::vector<pool::NodeHandle> m_nodes;
    std::vector<InstancePhysics> m_physics;
};

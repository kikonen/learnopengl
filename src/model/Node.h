#pragma once

#include <vector>
#include <array>
#include <functional>
#include <memory>

#include <glm/glm.hpp>

#include "ki/limits.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "audio/size.h"

#include "model/NodeState.h"
#include "model/NodeFlags.h"

#include "registry/NodeRegistry.h"

namespace backend {
    struct DrawOptions;
}

namespace kigl {
    struct GLVertexArray;
}

namespace render {
    class Batch;
}

namespace mesh {
    struct LodMesh;
}

namespace particle {
    class ParticleGenerator;
}

namespace audio {
    struct Listener;
    struct Source;
}

class Program;
class CameraComponent;
class Light;
class NodeGenerator;

struct PrepareContext;
struct UpdateContext;
class RenderContext;

class Registry;
class EntityRegistry;

struct Snapshot;

class Node final
{
    friend struct pool::NodeHandle;
    friend class NodeRegistry;

public:
    Node();
    Node(Node& o) = delete;
    Node(const Node&) = delete;
    Node(Node&& o) noexcept;
    ~Node();

    Node& operator=(Node& o) = delete;
    Node& operator=(Node&& o) noexcept;

    bool operator==(const Node& o) const noexcept
    {
        return m_handle == o.m_handle;
    }

    std::string str() const noexcept;

    inline ki::node_id getId() const noexcept { return m_handle.m_id; }
    inline pool::NodeHandle toHandle() const noexcept { return m_handle; }

    inline mesh::MeshType* getType() const noexcept
    {
        return m_typeHandle.toType();
    }

    const std::string& getName() const noexcept { return m_name; }
    void setName(std::string_view name) noexcept {
        m_name = name;
    }

    void prepareWT(
        const PrepareContext& ctx,
        NodeState& state);

    void prepareRT(
        const PrepareContext& ctx);

    void updateVAO(const RenderContext& ctx) noexcept;

    void bindBatch(
        const RenderContext& ctx,
        mesh::MeshType* type,
        const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
        uint8_t kindBits,
        render::Batch& batch) noexcept;

    inline pool::NodeHandle getParentHandle() const noexcept
    {
        return NodeRegistry::get().getParentHandle(m_entityIndex);
    }

    inline const Node* getParent() const noexcept
    {
        return NodeRegistry::get().getParent(m_entityIndex);
    }

    inline NodeState& modifyState() const noexcept
    {
        return NodeRegistry::get().modifyState(m_entityIndex);
    }

    inline const NodeState& getState() const noexcept
    {
        return NodeRegistry::get().getState(m_entityIndex);
    }

    //inline const Snapshot* getSnapshotWT() const noexcept
    //{
    //    return NodeRegistry::get().getSnapshotWT(m_entityIndex);
    //}

    inline const Snapshot* getSnapshotRT() const noexcept
    {
        return NodeRegistry::get().getSnapshotRT(m_entityIndex);
    }

    audio::Source* getAudioSource(audio::source_id) const;

    //void updateModelMatrix() noexcept;

    bool isEntity() const noexcept;

    //inline int getCloneIndex() const noexcept {
    //    return m_cloneIndex;
    //}

    //inline void setCloneIndex(int cloneIndex) {
    //    m_cloneIndex = cloneIndex;
    //}

    template<typename T>
    T* getGenerator()
    {
        return dynamic_cast<T*>(m_generator.get());
    }

public:
    ki::node_id lua_get_id() const noexcept;
    const std::string& lua_get_name() const noexcept;

    int lua_get_clone_index() const noexcept;

    const glm::vec3& lua_get_pos() const noexcept;

public:
    pool::TypeHandle m_typeHandle{};

    std::unique_ptr<CameraComponent> m_camera{ nullptr };
    std::unique_ptr<Light> m_light{ nullptr };
    std::unique_ptr<particle::ParticleGenerator> m_particleGenerator{ nullptr };

    std::unique_ptr<NodeGenerator> m_generator{ nullptr };

    // Index of node in NodeRegistry
    uint32_t m_entityIndex{ 0 };

    std::unique_ptr<audio::Listener> m_audioListener;
    std::unique_ptr<std::vector<audio::Source>> m_audioSources;

    std::string m_name;

    ki::node_id m_ignoredBy{ 0 };

private:
    pool::NodeHandle m_handle;

    //int m_cloneIndex{ 0 };

public:
    NodeFlags m_flags;

    bool m_visible : 1 { true };
    bool m_preparedRT : 1 { false };
};

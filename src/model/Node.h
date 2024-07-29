#pragma once

#include <vector>
#include <array>
#include <functional>

#include <glm/glm.hpp>

#include "ki/limits.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "audio/size.h"

#include "model/NodeState.h"
#include "model/NodeFlags.h"

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
        const PrepareContext& ctx);

    void prepareRT(
        const PrepareContext& ctx);

    void updateVAO(const RenderContext& ctx) noexcept;

    void bindBatch(
        const RenderContext& ctx,
        mesh::MeshType* type,
        const std::function<Program* (const mesh::LodMesh&)>& programSelector,
        uint8_t kindBits,
        render::Batch& batch) noexcept;

    inline Node* getParent() const noexcept {
        return m_parent.toNode();
    }

    inline void setParent(pool::NodeHandle parent) noexcept {
        m_parent = parent;
    }

    inline void setParent(const Node* parent) noexcept {
        m_parent = parent;
    }

    inline void removeChild(pool::NodeHandle node) {
        // TODO KI
    }

    inline const NodeState& getState() const noexcept {
        return m_state;
    }

    inline NodeState& modifyState() noexcept {
        return m_state;
    }

    const Snapshot& getActiveSnapshot(Registry* registry) const noexcept;
    Snapshot& modifyActiveSnapshot(Registry* registry) noexcept;

    void updateModelMatrix() noexcept;

    bool isEntity() const noexcept;

    inline int getTagMaterialIndex() const noexcept { return m_tagMaterialIndex; }
    inline int getSelectionMaterialIndex() const noexcept { return m_selectionMaterialIndex; }

    void setTagMaterialIndex(int index);
    void setSelectionMaterialIndex(int index);

    // @return -1 if no highlight color
    int getHighlightIndex() const noexcept;

    inline int getCloneIndex() const noexcept {
        return m_cloneIndex;
    }

    inline void setCloneIndex(int cloneIndex) {
        m_cloneIndex = cloneIndex;
    }

    inline bool isHighlighted() const noexcept
    {
        return getHighlightIndex() != -1;
    }

    inline bool isSelected() const noexcept { return m_selectionMaterialIndex > -1; }
    inline bool isTagged() const noexcept { return m_tagMaterialIndex > -1; }

    template<typename T>
    T* getGenerator()
    {
        return dynamic_cast<T*>(m_generator.get());
    }

public:
    ki::node_id lua_getId() const noexcept;
    const std::string& lua_getName() const noexcept;

    int lua_getCloneIndex() const noexcept;

    const std::array<float, 3> lua_getPos() const noexcept;

public:
    pool::TypeHandle m_typeHandle{};

    uint32_t m_snapshotIndex{ 0 };
    uint32_t m_entityIndex{ 0 };

    std::unique_ptr<CameraComponent> m_camera{ nullptr };
    std::unique_ptr<Light> m_light{ nullptr };
    std::unique_ptr<particle::ParticleGenerator> m_particleGenerator{ nullptr };

    std::unique_ptr<NodeGenerator> m_generator{ nullptr };

    NodeGenerator* m_instancer{ nullptr };

    std::array<audio::source_id, ki::MAX_NODE_AUDIO_SOURCE> m_audioSourceIds{ 0, 0, 0, 0 };

    std::string m_name;

private:
    NodeState m_state;

    pool::NodeHandle m_handle;
    pool::NodeHandle m_parent;

    int m_cloneIndex{ 0 };

    int m_tagMaterialIndex{ -1 };
    int m_selectionMaterialIndex{ -1 };

public:
    NodeFlags m_flags;

    bool m_visible : 1 { true };
    bool m_preparedRT : 1 { false };
};

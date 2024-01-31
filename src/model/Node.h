#pragma once

#include <vector>
#include <array>

#include <glm/glm.hpp>

#include "ki/limits.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "audio/size.h"

#include "model/NodeTransform.h"
#include "model/Snapshot.h"

namespace backend {
    struct DrawOptions;
}

namespace kigl {
    struct GLVertexArray;
}

namespace render {
    class Batch;
}

class Camera;
class Light;
class ParticleGenerator;
class NodeGenerator;

struct PrepareContext;
struct UpdateContext;
class RenderContext;

class EntityRegistry;
class ParticleGenrator;


class Node final
{
    friend class pool::NodeHandle;
    friend class NodeRegistry;

public:
    Node();
    Node(ki::node_id id);
    Node(Node& o) = delete;
    Node(const Node&) = delete;
    Node(Node&& o) noexcept;
    ~Node();

    Node& operator=(Node& o) = delete;
    Node& operator=(Node&& o) noexcept;

    const std::string str() const noexcept;

    inline ki::node_id getId() const noexcept { return m_id; }
    inline uint32_t getHandleIndex() const noexcept { return m_handleIndex; }

    pool::NodeHandle toHandle() const noexcept {
        return { m_handleIndex, m_id };
    }

    void prepare(
        const PrepareContext& ctx);

    void updateVAO(const RenderContext& ctx) noexcept;
    const kigl::GLVertexArray* getVAO() const noexcept;
    const backend::DrawOptions& getDrawOptions() const noexcept;
    void bindBatch(const RenderContext& ctx, render::Batch& batch) noexcept;

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

    inline const NodeTransform& getTransform() const noexcept {
        return m_transform;
    }

    inline NodeTransform& modifyTransform() noexcept {
        return m_transform;
    }

    void updateModelMatrix() noexcept;

    bool isEntity() const noexcept;

    inline int getTagMaterialIndex() const noexcept { return m_tagMaterialIndex;  }
    inline int getSelectionMaterialIndex() const noexcept { return m_selectionMaterialIndex;  }

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

public:
    ki::node_id lua_getId() const noexcept;
    const std::string& lua_getName() const noexcept;

    int lua_getCloneIndex() const noexcept;

    const std::array<float, 3> lua_getPos() const noexcept;

public:
    std::array<audio::source_id, ki::MAX_NODE_AUDIO_SOURCE> m_audioSourceIds{ 0, 0, 0, 0 };

    bool m_visible : 1 { true };
    pool::TypeHandle m_typeHandle{};

    std::unique_ptr<Camera> m_camera{ nullptr };
    std::unique_ptr<Light> m_light{ nullptr };
    std::unique_ptr<ParticleGenerator> m_particleGenerator{ nullptr };

    std::unique_ptr<NodeGenerator> m_generator{ nullptr };

    NodeGenerator* m_instancer{ nullptr };

    uint32_t m_snapshotIndex{ 0 };
    uint32_t m_entityIndex{ 0 };

    bool m_preparedRT : 1 { false };

#ifdef _DEBUG
    std::string m_resolvedSID;
#endif

private:
    ki::node_id m_id{ 0 };
    uint32_t m_handleIndex{ 0 };

    pool::NodeHandle m_parent{};

    NodeTransform m_transform;

    int m_cloneIndex{ 0 };

    int m_tagMaterialIndex{ -1 };
    int m_selectionMaterialIndex{ -1 };
};

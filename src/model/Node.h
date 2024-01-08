#pragma once

#include <vector>
#include <array>

#include <glm/glm.hpp>

#include "ki/limits.h"

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

namespace mesh {
    class MeshType;
}

class Assets;

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
public:
    Node(const mesh::MeshType* type);
    ~Node();

    const std::string str() const noexcept;

    void prepare(
        const PrepareContext& ctx);

    void updateWT(const UpdateContext& ctx) noexcept;

    void updateEntity(
        const UpdateContext& ctx,
        EntityRegistry* entityRegistry);

    void updateVAO(const RenderContext& ctx) noexcept;
    const kigl::GLVertexArray* getVAO() const noexcept;
    const backend::DrawOptions& getDrawOptions() const noexcept;
    void bindBatch(const RenderContext& ctx, render::Batch& batch) noexcept;

    inline Node* getParent() {
        return m_parent;
    }

    inline void setParent(Node* parent) {
        m_parent = parent;
    }

    inline const std::vector<Node*>& getChildren() const
    {
        return m_children;
    }

    inline void addChild(Node* child) {
        m_children.emplace_back(child);
    }

    inline void removeChild(Node* node) {
        // TODO KI
    }

    inline const NodeTransform& getTransform() const noexcept {
        return m_transform;
    }

    inline NodeTransform& modifyTransform() noexcept {
        return m_transform;
    }

    void snapshotWT() noexcept;
    void snapshotRT() noexcept;

    inline const Snapshot& getSnapshot() const noexcept {
        return m_snapshotRT;
    }

    //inline Snapshot& modifySnapshot() noexcept {
    //    return m_snapshot;
    //}

    inline ki::size_t_entity_flags getEntityFlags() const noexcept {
         return m_entityFlags;
    }

    void updateModelMatrix() noexcept;

    bool isEntity() const noexcept;

    inline int getTagMaterialIndex() const noexcept { return m_tagMaterialIndex;  }
    inline int getSelectionMaterialIndex() const noexcept { return m_selectionMaterialIndex;  }

    void setTagMaterialIndex(int index);
    void setSelectionMaterialIndex(int index);

    // @return -1 if no highlight color
    int getHighlightIndex(const Assets& assets) const noexcept;

    inline int getCloneIndex() const noexcept {
        return m_cloneIndex;
    }

    inline void setCloneIndex(int cloneIndex) {
        m_cloneIndex = cloneIndex;
    }

    inline bool isHighlighted(const Assets & assets) const noexcept
    {
        return getHighlightIndex(assets) != -1;
    }

    inline bool isSelected() const noexcept { return m_selectionMaterialIndex > -1; }
    inline bool isTagged() const noexcept { return m_tagMaterialIndex > -1; }

public:
    ki::node_id lua_getId() const noexcept;
    const std::string& lua_getName() const noexcept;

    int lua_getCloneIndex() const noexcept;

    const std::array<float, 3> lua_getPos() const noexcept;

protected:

public:
    // *INTERNAL* LUID in scene
    // used for object identity in shader
    const ki::node_id m_id;

    std::array<audio::source_id, ki::MAX_NODE_AUDIO_SOURCE> m_audioSourceIds{ 0, 0, 0, 0 };

    bool m_visible{ true };
    // NOTE KI type needed with node for practicality reasons
    const mesh::MeshType* m_type{ nullptr };

    std::unique_ptr<Camera> m_camera{ nullptr };
    std::unique_ptr<Light> m_light{ nullptr };
    std::unique_ptr<ParticleGenerator> m_particleGenerator{ nullptr };

    std::unique_ptr<NodeGenerator> m_generator{ nullptr };

    NodeGenerator* m_instancer{ nullptr };

private:
    Node* m_parent{ nullptr };
    std::vector<Node*> m_children;

    NodeTransform m_transform;
    bool m_forceUpdateEntity{ true };
    ki::size_t_entity_flags m_entityFlags{ 0 };

    Snapshot m_snapshotWT;
    Snapshot m_snapshotRT;
    bool m_forceUpdateSnapshot{ true };

    int m_cloneIndex{ 0 };

    int m_tagMaterialIndex{ -1 };
    int m_selectionMaterialIndex{ -1 };
};

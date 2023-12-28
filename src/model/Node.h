#pragma once

#include <array>

#include <glm/glm.hpp>

#include "ki/limits.h"

#include "asset/Assets.h"

#include "audio/size.h"

#include "model/NodeTransform.h"

class Camera;
class Light;
class ParticleGenerator;
class NodeGenerator;

class UpdateContext;
class RenderContext;

class MeshType;
class Registry;
class EntityRegistry;
class ParticleGenrator;
class Batch;


class Node final
{
public:
    Node(const MeshType* type);
    ~Node();

    const std::string str() const noexcept;

    void prepare(
        const Assets& assets,
        Registry* registry);

    void update(const UpdateContext& ctx) noexcept;

    void updateEntity(
        const UpdateContext& ctx,
        EntityRegistry* entityRegistry);

    void bindBatch(const RenderContext& ctx, Batch& batch) noexcept;

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

    inline NodeTransform& getTransform() noexcept {
        return m_transform;
    }

    inline ki::size_t_entity_flags getEntityFlags() const noexcept {
         return m_entityFlags;
    }

    inline int getEntityIndex() const noexcept {
        return m_transform.m_entityIndex;
    }

    inline const glm::vec3& getWorldPosition() const noexcept {
        return m_transform.getWorldPosition();
    }

    inline const glm::vec4 getVolume() const noexcept {
        return m_transform.getVolume();
    }

    void setVolume(const glm::vec4& volume) {
        m_transform.setVolume(volume);
    }

    inline const glm::vec3& getFront() const noexcept {
        return m_transform.getFront();
    }

    void setFront(const glm::vec3& front) noexcept
    {
        m_transform.setFront(front);
    }

    inline void setPosition(const glm::vec3& pos) noexcept {
        m_transform.setPosition(pos);
    }

    inline void adjustPosition(const glm::vec3& adjust) noexcept {
        m_transform.adjustPosition(adjust);
    }

    inline const glm::vec3 getPosition() const noexcept {
        return m_transform.getPosition();
    }

    inline ki::level_id getParentMatrixLevel() const noexcept {
        return m_transform.m_parentMatrixLevel;
    }

    inline ki::level_id getMatrixLevel() const noexcept {
        return m_transform.m_matrixLevel;
    }

    inline const glm::mat4& getModelMatrix() const noexcept {
        return m_transform.m_modelMatrix;
    }

    void updateModelMatrix() noexcept;

    bool isEntity() const noexcept;

    inline int getTagMaterialIndex() const { return m_tagMaterialIndex;  }
    inline int getSelectionMaterialIndex() const { return m_selectionMaterialIndex;  }

    void setTagMaterialIndex(int index);
    void setSelectionMaterialIndex(int index);

    // @return -1 if no highlight color
    inline int getHighlightIndex(const Assets& assets) const
    {
        if (assets.showHighlight) {
            if (assets.showTagged && m_tagMaterialIndex > -1) return m_tagMaterialIndex;
            if (assets.showSelection && m_selectionMaterialIndex > -1) return m_selectionMaterialIndex;
        }
        return -1;
    }

    inline int getCloneIndex() const {
        return m_cloneIndex;
    }

    inline void setCloneIndex(int cloneIndex) {
        m_cloneIndex = cloneIndex;
    }

    inline bool isHighlighted(const Assets & assets) const
    {
        return getHighlightIndex(assets) != -1;
    }

    inline bool isSelected() { return m_selectionMaterialIndex > -1; }
    inline bool isTagged() { return m_tagMaterialIndex > -1; }

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
    const MeshType* m_type{ nullptr };

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

    int m_cloneIndex{ 0 };

    int m_tagMaterialIndex = -1;
    int m_selectionMaterialIndex = -1;
};

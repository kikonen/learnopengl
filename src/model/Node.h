#pragma once

#include <array>

#include <glm/glm.hpp>
#include <ki/uuid.h>

#include "ki/limits.h"

#include "asset/Assets.h"

#include "audio/size.h"

#include "model/NodeInstance.h"

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

struct EntitySSBO;

namespace physics {
    struct Object;
}

class Node final
{
public:
    Node(MeshType* type);
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

    inline NodeInstance& getInstance() noexcept {
        return m_instance;
    }

    inline int getEntityIndex() const noexcept {
        return m_instance.m_entityIndex;
    }

    inline const glm::vec3 getWorldPosition() const noexcept {
        return m_instance.getWorldPosition();
    }

    inline const glm::vec3& getViewUp() const noexcept {
        return m_instance.getViewUp();
    }

    inline const glm::vec3& getViewFront() const noexcept {
        return m_instance.getViewFront();
    }

    inline const glm::vec3& getViewRight() const noexcept {
        return m_instance.getViewRight();
    }

    inline const glm::vec4 getVolume() const noexcept {
        return m_instance.getVolume();
    }

    void setVolume(const glm::vec4& volume) {
        m_instance.setVolume(volume);
    }

    inline const glm::vec3& getFront() const noexcept {
        return m_instance.getFront();
    }

    void setFront(const glm::vec3& front) noexcept
    {
        m_instance.setFront(front);
    }

    inline void setPosition(const glm::vec3& pos) noexcept {
        m_instance.setPosition(pos);
    }

    inline void adjustPosition(const glm::vec3& adjust) noexcept {
        m_instance.adjustPosition(adjust);
    }

    inline const glm::vec3 getPosition() const noexcept {
        return m_instance.getPosition();
    }

    inline void setBaseRotation(const glm::quat& rot) noexcept {
        m_instance.setBaseRotation(rot);
    }

    inline void setQuatRotation(const glm::quat& rot) noexcept {
        m_instance.setQuatRotation(rot);
    }

    inline void adjustQuatRotation(const glm::quat& adjust) noexcept {
        m_instance.adjustQuatRotation(adjust);
    }

    inline void setDegreesRotation(const glm::vec3& rot) noexcept {
        m_instance.setDegreesRotation(rot);
    }

    inline void adjustDegreesRotation(const glm::vec3& adjust) noexcept {
        m_instance.adjustDegreesRotation(adjust);
    }

    inline const glm::vec3& getDegreesRotation() const noexcept {
        return m_instance.getDegreesRotation();
    }

    inline const glm::quat& getQuatRotation() const noexcept {
        return m_instance.getQuatRotation();
    }

    inline void setScale(float scale) noexcept {
        m_instance.setScale(scale);
    }

    inline void setScale(const glm::vec3& scale) noexcept {
        m_instance.setScale(scale);
    }

    inline void adjustScale(const glm::vec3& adjust) noexcept {
        m_instance.adjustScale(adjust);
    }

    inline const glm::vec3 getScale() const noexcept {
        return m_instance.getScale();
    }

    inline int getParentMatrixLevel() const noexcept {
        return m_instance.m_parentMatrixLevel;
    }

    inline int getMatrixLevel() const noexcept {
        return m_instance.m_matrixLevel;
    }

    inline const glm::mat4& getModelMatrix() const noexcept {
        return m_instance.m_modelMatrix;
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
    ki::object_id lua_getId() const noexcept;
    const std::string& lua_getName() const noexcept;

    int lua_getCloneIndex() const noexcept;

        const std::array<float, 3> lua_getPos() const noexcept;

protected:

public:
    // *INTERNAL* LUID in scene
    // used for object identity in shader
    const ki::object_id m_id;

    // UUID of node for persistency
    // => *CAN* be empty for auto generated nodes
    uuids::uuid m_uuid;

    audio::listener_id m_audioListenerId{ 0 };
    std::array<audio::source_id, ki::MAX_NODE_AUDIO_SOURCE> m_audioSourceIds;

    // NOTE KI type needed with node for practicality reasons
    MeshType* m_type{ nullptr };

    std::unique_ptr<Camera> m_camera{ nullptr };
    std::unique_ptr<Light> m_light{ nullptr };
    std::unique_ptr<ParticleGenerator> m_particleGenerator{ nullptr };

    std::unique_ptr<NodeGenerator> m_generator{ nullptr };

    std::unique_ptr<physics::Object> m_physics{ nullptr };

    NodeGenerator* m_instancer{ nullptr };

protected:
    bool m_prepared = false;

private:
    Node* m_parent{ nullptr };

    NodeInstance m_instance;

    int m_cloneIndex{ 0 };

    int m_tagMaterialIndex = -1;
    int m_selectionMaterialIndex = -1;
};

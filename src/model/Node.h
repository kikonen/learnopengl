#pragma once

#include <glm/glm.hpp>
#include <ki/uuid.h>

#include "model/NodeInstance.h"

#include "generator/NodeGenerator.h"

#include "registry/MeshType.h"


class Camera;
class Light;
class ParticleGenerator;

class UpdateContext;
class RenderContext;

class MeshType;
class EntityRegistry;
class ParticleGenrator;
class Batch;

struct EntitySSBO;

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

    inline const glm::vec3& getViewFront() const noexcept {
        return m_viewFront;
    }

    inline const glm::vec4& getVolume() const noexcept {
        return m_instance.getVolume();
    }

    void setVolume(const glm::vec4& volume) {
        m_instance.setVolume(volume);
    }

    inline const glm::vec3& getFront() const noexcept {
        return m_front;
    }

    void setFront(const glm::vec3& front) noexcept
    {
        m_front = glm::normalize(front);
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

    inline void setRotation(const glm::vec3& rotation) noexcept {
        m_instance.setRotation(rotation);
    }

    inline void adjustRotation(const glm::vec3& adjust) noexcept {
        m_instance.adjustRotation(adjust);
    }

    inline const glm::vec3& getRotation() const noexcept {
        return m_instance.getRotation();
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

    inline bool isEntity() {
        return m_type->getMesh() &&
            !m_type->m_flags.invisible;
    }

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

    inline int getMaterialIndex() const {
        return m_type->getMaterialIndex();
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

    static int nextID() noexcept;

public:
    int lua_getId() const noexcept;
    const std::string& lua_getName() const noexcept;

    int lua_getCloneIndex() const noexcept;

    const std::array<float, 3> lua_getPos() const noexcept;

protected:

public:
    // *INTERNAL* LUID in scene
    // used for object identity in shader
    const int m_objectID;

    // UUID of node for persistency
    // => *CAN* be empty for auto generated nodes
    uuids::uuid m_id;

    // NOTE KI type needed with node for practicality reasons
    MeshType* m_type{ nullptr };

    std::unique_ptr<Camera> m_camera{ nullptr };
    std::unique_ptr<Light> m_light{ nullptr };
    std::unique_ptr<ParticleGenerator> m_particleGenerator{ nullptr };

    std::unique_ptr<NodeGenerator> m_generator{ nullptr };

    NodeGenerator* m_instancer{ nullptr };

protected:
    bool m_prepared = false;

private:
    glm::vec3 m_viewFront{ 0.f, 0.f, 1.f };

    glm::vec3 m_front{ 0.f, 0.f, 1.f };

    Node* m_parent{ nullptr };

    NodeInstance m_instance;

    int m_cloneIndex{ 0 };

    int m_tagMaterialIndex = -1;
    int m_selectionMaterialIndex = -1;
};

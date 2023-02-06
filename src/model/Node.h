#pragma once

#include <glm/glm.hpp>
#include <ki/uuid.h>

#include "asset/AABB.h"

#include "registry/MeshType.h"


class NodeController;
class Camera;
class Light;
class ParticleGenerator;
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

    void update(const RenderContext& ctx, Node* parent) noexcept;

    void bindBatch(const RenderContext& ctx, Batch& batch) noexcept;

    inline const glm::vec3& getWorldPosition() const noexcept {
        return m_worldPosition;
    }

    inline const glm::vec3& getWorldPlaneNormal() const noexcept {
        return m_worldPlaneNormal;
    }

    inline const glm::vec4& getVolume() const noexcept {
        return m_aabb.getVolume();
    }

    bool inFrustum(const RenderContext& ctx, float radiusFlex) const;

    inline int getMatrixLevel() const noexcept {
        return m_matrixLevel;
    }

    void setPlaneNormal(const glm::vec3& planeNormal) noexcept;

    inline const glm::vec3& getPlaneNormal() const noexcept {
        return m_planeNormal;
    }

    void setPosition(const glm::vec3& pos) noexcept;

    inline const glm::vec3& getPosition() const noexcept {
        return m_position;
    }

    void setRotation(const glm::vec3& rotation) noexcept;

    inline const glm::vec3& getRotation() const noexcept {
        return m_rotation;
    }

    void setScale(float scale) noexcept;
    void setScale(const glm::vec3& scale) noexcept;

    inline const glm::vec3& getScale() const noexcept {
        return m_scale;
    }


    inline const glm::mat4& getModelMatrix() const noexcept {
        return m_modelMatrix;
    }

    void updateModelMatrix(Node* parent) noexcept;

    void setAABB(const AABB& aabb);

    const AABB& getAABB() const {
        return m_aabb;
    }

    inline bool isEntity() {
        return m_type->getMesh() &&
            !m_type->m_flags.noRender;
    }

    inline int getTagMaterialIndex() const { return m_tagMaterialIndex;  }
    inline int getSelectionMaterialIndex() const { return m_selectionMaterialIndex;  }

    void setTagMaterialIndex(int index);
    void setSelectionMaterialIndex(int index);

    // @return -1 if no highlight color
    int getHighlightIndex(const RenderContext& ctx) const;

    inline int getMaterialIndex() const {
        return m_type->getMaterialIndex();
    }

    inline bool isHighlighted() { return m_tagMaterialIndex > -1 || m_selectionMaterialIndex > -1; }

    inline bool isSelected() { return m_selectionMaterialIndex > -1; }
    inline bool isTagged() { return m_tagMaterialIndex > -1; }

    static int nextID() noexcept;

    void setEntityRange(int instancedIndex, int instancedCount) noexcept {
        m_instancedIndex = instancedIndex;
        m_instancedCount = instancedCount;
    }

public:
    int lua_getId() const noexcept;

    const std::array<float, 3> lua_getPos() const noexcept;
    void lua_setPos(float x, float y, float z) noexcept;

protected:

public:
    // *INTERNAL* LUID in scene
    // used for object identity in shader
    const int m_objectID;

    // UUID of node for persistency
    // => *CAN* be empty for auto generated nodes
    uuids::uuid m_id;

    // UUID of node's parent
    // => both parent & children queried from NodeRegistry
    uuids::uuid m_parentId;

    // NOTE KI group != parent (does not affect modelMatrix and such)
    uuids::uuid m_groupId;

    // NOTE KI type needed with node for practicality reasons
    MeshType* m_type{ nullptr };

    bool m_allowNormals = true;

    std::unique_ptr <NodeController> m_controller{ nullptr };

    std::unique_ptr<Camera> m_camera{ nullptr };
    std::unique_ptr<Light> m_light{ nullptr };
    std::unique_ptr<ParticleGenerator> m_particleGenerator{ nullptr };

protected:
    bool m_prepared = false;

    int m_entityIndex = -1;

    int m_instancedIndex = -1;
    int  m_instancedCount = 0;

    AABB m_aabb;

private:
    int m_matrixLevel = -1;
    int m_parentMatrixLevel = -1;

    glm::vec3 m_worldPosition{ 0.f };

    glm::vec3 m_worldPlaneNormal{ 0.f };

    glm::vec3 m_planeNormal{ 0 };

    glm::vec3 m_position{ 0.f };
    glm::vec3 m_rotation{ 0.f };

    glm::vec3 m_scale{ 1.f };

    glm::mat4 m_modelMatrix{ 1.f };
    glm::mat3 m_normalMatrix{ 1.f };

    glm::mat4 m_rotationMatrix{ 0.f };
    glm::mat4 m_translateMatrix{ 0.f };
    glm::mat4 m_scaleMatrix{ 0.f };

    int m_tagMaterialIndex = -1;
    int m_selectionMaterialIndex = -1;

    bool m_dirtyRotation = true;
    bool m_dirtyTranslate = true;
    bool m_dirtyScale = true;
    bool m_dirtyEntity = true;
};

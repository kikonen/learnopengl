#pragma once

#include <glm/glm.hpp>
#include <ki/uuid.h>

#include "asset/AABB.h"

#include "generator/NodeGenerator.h"

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

    void updateEntity(
        const RenderContext& ctx,
        EntityRegistry* entityRegistry);

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
        return { m_translateMatrix[3][0], m_translateMatrix[3][1], m_translateMatrix[3][2] };
    }

    void setRotation(const glm::vec3& rotation) noexcept;

    inline const glm::vec3& getRotation() const noexcept {
        return m_rotation;
    }

    void setScale(float scale) noexcept;
    void setScale(const glm::vec3& scale) noexcept;

    inline const glm::vec3& getScale() const noexcept {
        return { m_scaleMatrix[0][0], m_scaleMatrix[1][1], m_scaleMatrix[2][2] };
    }

    int getCloneIndex() {
        return m_cloneIndex;
    }

    void setCloneIndex(int cloneIndex) {
        m_cloneIndex = cloneIndex;
    }

    const glm::uvec3& getTile() {
        return m_tile;
    }

    void setTile(const glm::uvec3& tile) {
        m_tile = tile;
    }

    inline const glm::mat4& getModelMatrix() const noexcept {
        return m_modelMatrix;
    }

    void updateModelMatrix(Node* parent) noexcept;

    inline int getInstancedIndex() {
        return m_instancedIndex;
    }

    inline int getInstancedCount() {
        return m_instancedCount;
    }

    void setAABB(const AABB& aabb);

    const AABB& getAABB() const {
        return m_aabb;
    }

    inline bool isEntity() {
        return m_type->getMesh() &&
            !m_type->m_flags.invisible;
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
    const std::string& lua_getName() const noexcept;

    int lua_getCloneIndex() const noexcept;
    const std::array<unsigned int, 3> lua_getTile() const noexcept;

    const std::array<float, 3> lua_getPos() const noexcept;

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

    // NOTE KI type needed with node for practicality reasons
    MeshType* m_type{ nullptr };

    std::unique_ptr <NodeController> m_controller{ nullptr };

    std::unique_ptr<Camera> m_camera{ nullptr };
    std::unique_ptr<Light> m_light{ nullptr };
    std::unique_ptr<ParticleGenerator> m_particleGenerator{ nullptr };

    std::unique_ptr<NodeGenerator> m_generator{ nullptr };

    int m_entityIndex = -1;

protected:
    bool m_prepared = false;

    int m_instancedIndex = -1;
    int  m_instancedCount = 0;

    AABB m_aabb;

private:
    int m_matrixLevel = -1;
    int m_parentMatrixLevel = -1;

    glm::vec3 m_worldPosition{ 0.f };

    glm::vec3 m_worldPlaneNormal{ 0.f };

    glm::vec3 m_planeNormal{ 0 };

    glm::mat4 m_translateMatrix{ 1.f };
    glm::mat4 m_scaleMatrix{ 1.f };

    glm::vec3 m_rotation{ 0.f };
    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    // quaternion rotation matrix
    glm::mat4 m_rotationMatrix{ 1.f };

    int m_cloneIndex{ 0 };
    glm::uvec3 m_tile{ 0 };

    glm::mat4 m_modelMatrix{ 1.f };

    int m_tagMaterialIndex = -1;
    int m_selectionMaterialIndex = -1;

    bool m_dirty = true;
    bool m_dirtyEntity = true;
};

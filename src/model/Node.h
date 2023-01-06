#pragma once

#include <glm/glm.hpp>
#include <ki/uuid.h>

#include "asset/AABB.h"
#include "asset/OBB.h"

#include "asset/Sphere.h"
#include "component/ParticleGenerator.h"

#include "registry/MeshType.h"

#include "scene/Batch.h"

#include "component/Light.h"

class NodeController;
class Camera;
class RenderContext;
class MeshType;

class Node
{
public:
    Node(MeshType* type);
    virtual ~Node();

    const std::string str() const noexcept;

    virtual void prepare(const Assets& assets) noexcept;

    virtual void update(const RenderContext& ctx, Node* parent) noexcept;
    virtual void bindBatch(const RenderContext& ctx, Batch& batch) noexcept;

    const glm::vec3& getWorldPos() const noexcept;
    const glm::vec3& getWorldPlaneNormal() const noexcept;

    int getMatrixLevel() const noexcept;
    const glm::mat4& getWorldModelMatrix() const noexcept;
    const glm::mat3& getWorldNormalMatrix() const noexcept;

    void setPlaneNormal(const glm::vec3& planeNormal) noexcept;
    const glm::vec3& getPlaneNormal() const noexcept;

    void setPosition(const glm::vec3& pos) noexcept;
    const glm::vec3& getPosition() const noexcept;

    void setRotation(const glm::vec3& rotation) noexcept;
    const glm::vec3& getRotation() const noexcept;

    void setScale(float scale) noexcept;
    void setScale(const glm::vec3& scale) noexcept;
    const glm::vec3& getScale() const noexcept;

    const glm::mat4& getModelMatrix() const noexcept;

    virtual void updateModelMatrix(Node* parent) noexcept;

    const Volume* getVolume() const noexcept;
    void setVolume(std::unique_ptr<Volume> volume) noexcept;

    void setAABB(const AABB& aabb);
    const AABB& getAABB() const;

    OBB& getOBB();

    // @return -1 if no highlight color
    int getHighlightColor(const RenderContext& ctx) const;

    bool isHighlighted() { return m_tagMaterialIndex > -1 || m_selectionMaterialIndex > -1; }

    bool isSelected() { return m_selectionMaterialIndex > -1; }
    bool isTagged() { return m_tagMaterialIndex > -1; }

    static int nextID() noexcept;

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

    int m_tagMaterialIndex = -1;
    int m_selectionMaterialIndex = -1;
    bool m_allowNormals = true;

    std::unique_ptr <NodeController> m_controller{ nullptr };

    std::unique_ptr<Camera> m_camera{ nullptr };
    std::unique_ptr <Light> m_light{ nullptr };
    std::unique_ptr<ParticleGenerator> m_particleGenerator{ nullptr };

protected:
    bool m_prepared = false;

    std::unique_ptr<Volume> m_volume;
    AABB m_aabb;
    OBB m_obb;

private:
    int m_matrixLevel = -1;
    int m_parentMatrixLevel = -1;

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

    bool m_dirtyRotation = true;
    bool m_dirtyTranslate = true;
    bool m_dirtyScale = true;
};

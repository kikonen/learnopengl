#pragma once

#include <glm/glm.hpp>
#include <ki/uuid.h>

#include "asset/Sphere.h"
#include "component/ParticleGenerator.h"
#include "scene/NodeType.h"
#include "scene/RenderContext.h"
#include "scene/Batch.h"

#include "component/Light.h"

class NodeController;
class Camera;

class Node
{
public:
    Node(std::shared_ptr<NodeType> type);
    virtual ~Node();

    const std::string str() const;

    virtual void prepare(const Assets& assets);

    virtual void update(const RenderContext& ctx, Node* parent);
    virtual void bind(const RenderContext& ctx, Shader* shader);
    void bindBatch(const RenderContext& ctx, Batch& batch);
    virtual void draw(const RenderContext& ctx);

    const glm::vec3& getWorldPos() const;
    const glm::vec3& getWorldPlaneNormal() const;

    int getMatrixLevel() const;
    const glm::mat4& getWorldModelMatrix() const;
    const glm::mat3& getWorldNormalMatrix() const;

    void setPlaneNormal(const glm::vec3& planeNormal);
    const glm::vec3& getPlaneNormal() const;

    void setPosition(const glm::vec3& pos);
    const glm::vec3& getPosition() const;

    void setRotation(const glm::vec3& rotation);
    const glm::vec3& getRotation() const;

    void setScale(float scale);
    void setScale(const glm::vec3& scale);
    const glm::vec3& getScale() const;

    const glm::mat4& getModelMatrix() const;

    virtual void updateModelMatrix(Node* parent);

    const Volume* getVolume() const;
    void setVolume(std::unique_ptr<Volume> volume);

    static int nextID();

public:
    int lua_getId() const;

    const std::array<float, 3> lua_getPos() const;
    void lua_setPos(float x, float y, float z);

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
    std::shared_ptr<NodeType> m_type{ nullptr };

    bool m_selected = false;
    bool m_allowNormals = true;

    std::unique_ptr <NodeController> m_controller{ nullptr };

    std::unique_ptr<Camera> m_camera{ nullptr };
    std::unique_ptr <Light> m_light{ nullptr };
    std::unique_ptr<ParticleGenerator> m_particleGenerator{ nullptr };

protected:
    bool m_prepared = false;

    std::unique_ptr<Volume> m_volume;

private:
    int m_matrixLevel = 0;
    int m_parentMatrixLevel = 0;

    glm::vec3 m_worldPos{ 0.f };
    glm::vec3 m_worldPlaneNormal{ 0.f };

    glm::mat4 m_worldModelMatrix{ 1.f };
    glm::mat3 m_worldNormalMatrix{ 1.f };

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

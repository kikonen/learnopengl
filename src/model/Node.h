#pragma once

#include <glm/glm.hpp>
#include <stduuid/uuid.h>

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

    virtual void prepare(const Assets& assets);

    virtual void update(const RenderContext& ctx, Node* parent);
    virtual void bind(const RenderContext& ctx, Shader* shader);
    void bindBatch(const RenderContext& ctx, Batch& batch);
    virtual void draw(const RenderContext& ctx);

    const glm::vec3& getWorldPos();
    const glm::mat4& getWorldModelMatrix();
    const glm::mat3& getWorldNormalMatrix();

    const glm::mat4& getWorldModelMatrixNoScale();
    const glm::mat3& getWorldNormalMatrixNoScale();

    void setPos(const glm::vec3& pos);
    const glm::vec3& const getPos();

    void setRotation(const glm::vec3& rotation);
    const glm::vec3& getRotation();

    void setScale(float scale);
    void setScale(const glm::vec3& scale);
    const glm::vec3& getScale();

    const glm::mat4& getModelMatrix();
    virtual const Volume* getVolume();

    static int nextID();

protected:
    virtual void updateModelMatrix(Node* parent);

public:
    // *INTERNAL* LUID in scene
    // used for object identity in shader
    const int objectID;

    // UUID of node for persistency
    // => *CAN* be empty for auto generated nodes
    uuids::uuid id;

    // UUID of node's parent
    // => both parent & children queried from NodeRegistry
    uuids::uuid parentId;

    // NOTE KI group != parent (does not affect modelMatrix and such)
    uuids::uuid groupId;

    // NOTE KI type needed with node for practicality reasons
    std::shared_ptr<NodeType> type{ nullptr };

    bool selected = false;
    bool allowNormals = true;

    std::unique_ptr <NodeController> controller{ nullptr };

    std::unique_ptr<Camera> camera{ nullptr };
    std::unique_ptr <Light> light{ nullptr };
    std::unique_ptr<ParticleGenerator> particleGenerator{ nullptr };

protected:
    bool m_prepared = false;

private:
    glm::mat4 m_worldModelMatrix{ 1.f };
    glm::mat3 m_worldNormalMatrix{ 1.f };

    glm::mat4 m_worldModelMatrixNoScale{ 1.f };
    glm::mat3 m_worldNormalMatrixNoScale{ 1.f };

    glm::vec3 m_pos{ 0.f };
    glm::vec3 m_rotation{ 0.f };

    glm::vec3 m_scale{ 1.f };

    glm::mat4 m_modelMatrix{ 1.f };
    glm::mat3 m_normalMatrix{ 1.f };

    glm::mat4 m_modelMatrixNoScale{ 1.f };
    glm::mat3 m_normalMatrixNoScale{ 1.f };

    glm::mat4 m_rotationMatrix{ 0.f };
    glm::mat4 m_translateMatrix{ 0.f };
    glm::mat4 m_scaleMatrix{ 0.f };

    bool m_dirtyRotation = true;
    bool m_dirtyTranslate = true;
    bool m_dirtyScale = true;

    Batch m_singleBatch;
};

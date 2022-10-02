#include "Node.h"

#include <mutex>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/GL.h"

#include "controller/NodeController.h"

#include "scene/NodeRegistry.h"


namespace {
    const auto BASE_MAT_1 = glm::mat4(1.0f);

    int objectIDbase = 100;

    std::mutex object_id_lock;
}


int Node::nextID()
{
    std::lock_guard<std::mutex> lock(object_id_lock);
    return ++objectIDbase;
}

Node::Node(std::shared_ptr<NodeType> type)
    : type(type),
    objectID(nextID())
{
}

Node::~Node()
{
    KI_INFO_SB("NODE: delete type=" << type->typeID << " objectId=" << objectID);
}

void Node::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    if (controller) {
        controller->prepare(assets, *this);
    }
}

void Node::update(
    const RenderContext& ctx,
    Node* parent)
{
    //if (id == KI_UUID("7c90bc35-1a05-4755-b52a-1f8eea0bacfa")) KI_BREAK();

    if (parent && parent->id == KI_UUID("7c90bc35-1a05-4755-b52a-1f8eea0bacfa"))
        int x = 0;

    updateModelMatrix(parent);

    bool changed = true;
    if (controller) {
        changed = controller->update(ctx, *this, parent);
    }

    if (changed) 
        updateModelMatrix(parent);

    if (light) light->update(ctx, *this);

    NodeVector* children = ctx.registry.getChildren(*this);
    if (children) {
        for (auto& child : *children) {
            child->update(ctx, this);
        }
    }
}

void Node::bind(const RenderContext& ctx, Shader* shader)
{
}

void Node::bindBatch(const RenderContext& ctx, Batch& batch)
{
    batch.add(m_worldModelMatrix, m_worldModelMatrix, objectID);
}

void Node::draw(const RenderContext& ctx)
{
    // NOTE KI shader side supports *ONLY* instanced rendering
    m_singleBatch.batchSize = 1;
    m_singleBatch.prepare(type.get());

    m_singleBatch.draw(ctx, this, type->boundShader);
    //type->mesh->draw(ctx);
}

void Node::updateModelMatrix(Node* parent) {
    bool dirtyModel = m_dirtyRotation || m_dirtyTranslate || m_dirtyScale;

    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    if (m_dirtyRotation) {
        m_rotationMatrix = glm::toMat4(glm::quat(glm::radians(m_rotation)));
        m_dirtyRotation = false;
    }

    if (m_dirtyTranslate) {
        m_translateMatrix = glm::translate(
            BASE_MAT_1,
            m_pos
        );
        m_dirtyTranslate = false;
    }

    if (m_dirtyScale) {
        m_scaleMatrix = glm::scale(
            BASE_MAT_1,
            m_scale
        );
        m_dirtyScale = false;
    }

    if (dirtyModel) {
        m_modelMatrix = m_translateMatrix * m_rotationMatrix * m_scaleMatrix;

        // https://learnopengl.com/Lighting/Basic-Lighting
        // http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
        // normal = mat3(transpose(inverse(model))) * aNormal;
        m_normalMatrix = glm::transpose(glm::inverse(glm::mat3(m_modelMatrix)));

        m_modelMatrixNoScale = m_translateMatrix * m_rotationMatrix;
        m_normalMatrixNoScale = glm::transpose(glm::inverse(glm::mat3(m_modelMatrixNoScale)));
    }

    // NOTE KI *NOT* knowing if parent is changed
    // => thus have to ALWAYS recalculate
    if (parent) {
        m_worldModelMatrix = parent->m_worldModelMatrixNoScale * m_modelMatrix;
        m_worldNormalMatrix = parent->m_worldNormalMatrixNoScale * m_normalMatrix;

        m_worldModelMatrixNoScale = parent->m_worldModelMatrixNoScale * m_modelMatrixNoScale;
        m_worldNormalMatrixNoScale = parent->m_worldNormalMatrixNoScale * m_normalMatrixNoScale;
    }
    else {
        if (dirtyModel) {
            m_worldModelMatrix = m_modelMatrix;
            m_worldNormalMatrix = m_normalMatrix;

            m_worldModelMatrixNoScale = m_modelMatrixNoScale;
            m_worldNormalMatrixNoScale = m_normalMatrixNoScale;
        }
    }
 }

void Node::setPos(const glm::vec3& pos) {
    m_pos = pos;
    m_dirtyTranslate = true;
}

const glm::vec3&  const Node::getPos() {
    return m_pos;
}

void Node::setRotation(const glm::vec3& rotation) {
    m_rotation = rotation;
    m_dirtyRotation = true;
}

const glm::vec3&  Node::getRotation() {
    return m_rotation;
}

void Node::setScale(float scale) {
    m_scale.x = scale;
    m_scale.y = scale;
    m_scale.z = scale;
    m_dirtyScale = true;
}

void Node::setScale(const glm::vec3& scale)
{
    m_scale = scale;
    m_dirtyScale = true;
}

const glm::vec3& Node::getScale() {
    return m_scale;
}

const glm::mat4& Node::getModelMatrix()
{
    return m_modelMatrix;
}

const glm::mat4& Node::getWorldModelMatrix() {
    return m_worldModelMatrix;
}

const glm::mat3& Node::getWorldNormalMatrix() {
    return m_worldNormalMatrix;
}

const glm::mat4& Node::getWorldModelMatrixNoScale()
{
    return m_worldModelMatrixNoScale;
}

const glm::mat3& Node::getWorldNormalMatrixNoScale()
{
    return m_worldNormalMatrixNoScale;
}

const glm::vec3& const Node::getWorldPos() {
    return m_worldModelMatrix[3];
}


const Volume* Node::getVolume()
{
    return type->mesh ? type->mesh->m_volume.get() : nullptr;
}

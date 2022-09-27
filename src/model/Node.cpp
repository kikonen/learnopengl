#include "Node.h"

#include <mutex>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/GL.h"

#include "controller/NodeController.h"


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
    if (controller) {
        controller->prepare(assets, *this);
    }
}

bool Node::update(const RenderContext& ctx)
{
    if (!controller) return false;
    return controller->update(ctx, *this);
}

void Node::bind(const RenderContext& ctx, Shader* shader)
{
    updateModelMatrix();
}

void Node::bindBatch(const RenderContext& ctx, Batch& batch)
{
    updateModelMatrix();
    if (type->flags.mirror) {
        int x = 0;
    }
    batch.add(m_modelMat, m_normalMat, objectID);
}

void Node::draw(const RenderContext& ctx)
{
    // NOTE KI shader side supports *ONLY* instanced rendering
    m_singleBatch.batchSize = 1;
    m_singleBatch.prepare(type.get());

    m_singleBatch.draw(ctx, this, type->boundShader);
    //type->mesh->draw(ctx);
}

void Node::updateModelMatrix() {
    bool dirtyModel = m_dirtyRot || m_dirtyTrans || m_dirtyScale;

    if (!dirtyModel) {
        return;
    }

    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    if (m_dirtyRot) {
        m_rotMat = glm::toMat4(glm::quat(glm::radians(m_rotation)));
        m_dirtyRot = false;
    }

    if (m_dirtyTrans) {
        m_transMat = glm::translate(
            BASE_MAT_1,
            m_pos
        );
        m_dirtyTrans = false;
    }

    if (m_dirtyScale) {
        m_scaleMat = glm::scale(
            BASE_MAT_1,
            m_scale
        );
        m_dirtyScale = false;
    }

    if (dirtyModel) {
        m_modelMat = m_transMat * m_rotMat * m_scaleMat;

        // https://learnopengl.com/Lighting/Basic-Lighting
        // http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
        // normal = mat3(transpose(inverse(model))) * aNormal;
        m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));
    }
}

void Node::setPos(const glm::vec3& pos) {
    m_pos = pos;
    m_dirtyTrans = true;
    m_dirtyVolume = true;
}

const glm::vec3&  const Node::getPos() {
    return m_pos;
}

void Node::setRotation(const glm::vec3& rotation) {
    m_rotation = rotation;
    m_dirtyRot = true;
}

const glm::vec3&  Node::getRotation() {
    return m_rotation;
}

void Node::setScale(float scale) {
    m_scale.x = scale;
    m_scale.y = scale;
    m_scale.z = scale;
    m_dirtyScale = true;
    m_dirtyVolume = true;
}

void Node::setScale(const glm::vec3& scale)
{
    m_scale = scale;
    m_dirtyScale = true;
    m_dirtyVolume = true;
}

const glm::vec3& Node::getScale() {
    return m_scale;
}

const glm::mat4& Node::getModelMatrix()
{
    updateModelMatrix();
    return m_modelMat;
}

const Volume* Node::getVolume()
{
    return type->mesh->volume.get();
}

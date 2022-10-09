#include "Node.h"

#include <mutex>
#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/GL.h"

#include "controller/NodeController.h"

#include "scene/NodeRegistry.h"


namespace {
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
    KI_INFO_SB("NODE: delete " << str());
}

const std::string Node::str() const
{
    return fmt::format(
        "<NODEL: {} / {} - type={}>",
        objectID, KI_UUID_STR(id), type->str());
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
    updateModelMatrix(parent);

    bool changed = false;
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
    batch.add(m_worldModelMatrix, m_worldNormalMatrix, objectID);
}

void Node::draw(const RenderContext& ctx)
{
}

void Node::updateModelMatrix(Node* parent) {
    bool dirtyModel = m_dirtyRotation || m_dirtyTranslate || m_dirtyScale;

    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    if (m_dirtyRotation) {
        m_rotationMatrix = glm::toMat4(glm::quat(glm::radians(m_rotation)));
        m_dirtyRotation = false;
    }

    const auto BASE_MAT_1 = glm::mat4(1.0f);

    if (m_dirtyTranslate) {
        m_translateMatrix = glm::translate(
            BASE_MAT_1,
            m_pos
        );
        m_dirtyTranslate = false;
    }

    if (m_dirtyScale) {
        assert(m_scale.x >= 0 && m_scale.y >= 0 && m_scale.z >= 0);

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
    }

    // NOTE KI *NOT* knowing if parent is changed
    // => thus have to ALWAYS recalculate
    if (parent) {
        if (dirtyModel || m_parentMatrixLevel != parent->m_matrixLevel) {
            m_worldModelMatrix = parent->m_worldModelMatrix * m_modelMatrix;
            m_worldNormalMatrix = parent->m_worldNormalMatrix * m_normalMatrix;
            m_parentMatrixLevel = parent->m_matrixLevel;
            m_worldPos = m_worldModelMatrix[3];
            m_matrixLevel++;
        }
    }
    else {
        if (dirtyModel) {
            m_worldModelMatrix = m_modelMatrix;
            m_worldNormalMatrix = m_normalMatrix;
            m_worldPos = m_worldModelMatrix[3];
            m_matrixLevel++;
        }
    }
}

void Node::setPos(const glm::vec3& pos) {
    m_pos = pos;
    m_dirtyTranslate = true;
}

const glm::vec3& Node::getPos() const {
    return m_pos;
}

void Node::setRotation(const glm::vec3& rotation) {
    m_rotation = rotation;
    m_dirtyRotation = true;
}

const glm::vec3&  Node::getRotation() const {
    return m_rotation;
}

void Node::setScale(float scale) {
    assert(scale >= 0);
    m_scale.x = scale;
    m_scale.y = scale;
    m_scale.z = scale;
    m_dirtyScale = true;
}

void Node::setScale(const glm::vec3& scale)
{
    m_scale = scale;
    assert(m_scale.x >= 0 && m_scale.y >= 0 && m_scale.z >= 0);
    m_dirtyScale = true;
}

const glm::vec3& Node::getScale() const {
    return m_scale;
}

const glm::mat4& Node::getModelMatrix() const
{
    return m_modelMatrix;
}

const int const Node::getMatrixLevel() const {
    return m_matrixLevel;
}

const glm::mat4& Node::getWorldModelMatrix() const {
    return m_worldModelMatrix;
}

const glm::mat3& Node::getWorldNormalMatrix() const  {
    return m_worldNormalMatrix;
}

const glm::vec3& Node::getWorldPos() const  {
    return m_worldPos;
}

const Volume* Node::getVolume() 
{
    return type->mesh ? type->mesh->m_volume.get() : nullptr;
}

const std::array<float, 3> Node::lua_getPos() const
{
    return { m_pos.x, m_pos.y, m_pos.z };
}

void Node::lua_setPos(float x, float y, float z)
{
    m_pos.x = x;
    m_pos.y = y;
    m_pos.z = z;
    m_dirtyTranslate = true;
}

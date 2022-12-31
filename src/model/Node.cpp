#include "Node.h"

#include <mutex>
#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/GL.h"

#include "controller/NodeController.h"

#include "registry/NodeRegistry.h"

#include "scene/RenderContext.h"

namespace {
    int objectIDbase = 100;

    std::mutex object_id_lock;

}

int Node::nextID() noexcept
{
    std::lock_guard<std::mutex> lock(object_id_lock);
    return ++objectIDbase;
}

Node::Node(MeshType* type)
    : m_type(type),
    m_objectID(nextID())
{
}

Node::~Node()
{
    KI_INFO_SB("NODE: delete " << str());
}

const std::string Node::str() const noexcept
{
    return fmt::format(
        "<NODEL: {} / {} - type={}>",
        m_objectID, KI_UUID_STR(m_id), m_type->str());
}

void Node::prepare(const Assets& assets) noexcept
{
    if (m_prepared) return;
    m_prepared = true;

    if (m_controller) {
        m_controller->prepare(assets, *this);
    }
}

void Node::update(
    const RenderContext& ctx,
    Node* parent) noexcept
{
    updateModelMatrix(parent);

    bool changed = false;
    if (m_controller) {
        changed = m_controller->update(ctx, *this, parent);
    }

    if (changed)
        updateModelMatrix(parent);

    if (m_light) m_light->update(ctx, *this);

    const NodeVector* children = ctx.registry.getChildren(*this);
    if (children) {
        for (auto& child : *children) {
            child->update(ctx, this);
        }
    }
}

void Node::bindBatch(const RenderContext& ctx, Batch& batch) noexcept
{
    batch.add(ctx, m_modelMatrix, m_normalMatrix, m_objectID, getHighlightColor());
}

void Node::updateModelMatrix(Node* parent) noexcept
{
    int parentMatrixLevel = parent ? parent->m_matrixLevel : -1;
    bool dirtyModel = m_dirtyRotation
        || m_dirtyTranslate
        || m_dirtyScale
        || m_parentMatrixLevel != parentMatrixLevel;
    if (!dirtyModel) return;

    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    if (m_dirtyRotation) {
        m_rotationMatrix = glm::toMat4(glm::quat(glm::radians(m_rotation)));
        m_dirtyRotation = false;
    }

    const auto BASE_MAT_1 = glm::mat4(1.0f);

    if (m_dirtyTranslate) {
        m_translateMatrix = glm::translate(
            BASE_MAT_1,
            m_position
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

    // https://learnopengl.com/Lighting/Basic-Lighting
    // http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
    // https://stackoverflow.com/questions/27600045/the-correct-way-to-calculate-normal-matrix
    // normal = mat3(transpose(inverse(model))) * aNormal;

    if (parent) {
        m_modelMatrix = parent->m_modelMatrix * m_translateMatrix * m_rotationMatrix * m_scaleMatrix;
        m_normalMatrix = parent->m_normalMatrix * glm::inverseTranspose(glm::mat3(m_modelMatrix));
        m_parentMatrixLevel = parentMatrixLevel;
    }
    else {
        m_modelMatrix = m_translateMatrix * m_rotationMatrix * m_scaleMatrix;
        m_normalMatrix = glm::inverseTranspose(glm::mat3(m_modelMatrix));
    }

    m_worldPlaneNormal = glm::normalize(glm::vec3(m_rotationMatrix * glm::vec4(m_planeNormal, 1.0)));

    m_matrixLevel++;
}

void Node::setPlaneNormal(const glm::vec3& planeNormal) noexcept
{
    m_planeNormal = planeNormal;
    m_dirtyRotation = true;
}

const glm::vec3& Node::getPlaneNormal() const noexcept
{
    return m_planeNormal;
}

void Node::setPosition(const glm::vec3& pos) noexcept
{
    m_position = pos;
    m_dirtyTranslate = true;
}

const glm::vec3& Node::getPosition() const noexcept
{
    return m_position;
}

void Node::setRotation(const glm::vec3& rotation) noexcept
{
    m_rotation = rotation;
    m_dirtyRotation = true;
}

const glm::vec3&  Node::getRotation() const noexcept
{
    return m_rotation;
}

void Node::setScale(float scale) noexcept
{
    assert(scale >= 0);
    m_scale.x = scale;
    m_scale.y = scale;
    m_scale.z = scale;
    m_dirtyScale = true;
}

void Node::setScale(const glm::vec3& scale) noexcept
{
    m_scale = scale;
    assert(m_scale.x >= 0 && m_scale.y >= 0 && m_scale.z >= 0);
    m_dirtyScale = true;
}

const glm::vec3& Node::getScale() const noexcept
{
    return m_scale;
}

const glm::mat4& Node::getModelMatrix() const noexcept
{
    return m_modelMatrix;
}

int Node::getMatrixLevel() const noexcept
{
    return m_matrixLevel;
}

const glm::mat4& Node::getWorldModelMatrix() const noexcept
{
    return m_modelMatrix;
}

const glm::mat3& Node::getWorldNormalMatrix() const noexcept
{
    return m_normalMatrix;
}

const glm::vec3& Node::getWorldPos() const noexcept
{
    return m_modelMatrix[3];
}

const glm::vec3& Node::getWorldPlaneNormal() const noexcept
{
    return m_worldPlaneNormal;
}

const Volume* Node::getVolume() const noexcept
{
    return m_volume.get();
}

void Node::setVolume(std::unique_ptr<Volume> volume) noexcept
{
    m_volume = std::move(volume);
}

void Node::setAABB(const AABB& aabb)
{
    m_aabb = aabb;
    m_obb = { aabb };
}

const AABB& Node::getAABB() const
{
    return m_aabb;
}

OBB& Node::getOBB()
{
    return m_obb;
}

int Node::getHighlightColor() const
{
    if (m_tagMaterialIndex > -1) return m_tagMaterialIndex;
    if (m_selectionMaterialIndex > -1) return m_selectionMaterialIndex;
    return -1;
}

int Node::lua_getId() const noexcept
{
    return m_objectID;
}

const std::array<float, 3> Node::lua_getPos() const noexcept
{
    return { m_position.x, m_position.y, m_position.z };
}

void Node::lua_setPos(float x, float y, float z) noexcept
{
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;
    m_dirtyTranslate = true;
}

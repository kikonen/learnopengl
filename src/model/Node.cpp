#include "Node.h"

#include <mutex>
#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/GL.h"

#include "asset/MaterialIndex.h"

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"

#include "controller/NodeController.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/EntitySSBO.h"

#include "scene/RenderContext.h"
#include "scene/Batch.h"

namespace {
    int objectIDbase = 100;

    std::mutex object_id_lock;

    const static glm::mat4 IDENTITY_MATRIX{ 1.f };
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
    KI_INFO(fmt::format("NODE: delete - {}", str()));
}

const std::string Node::str() const noexcept
{
    return fmt::format(
        "<NODE: {} / {} - entity={}, type={}>",
        m_objectID, KI_UUID_STR(m_id), m_entityIndex, m_type->str());
}

void Node::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    {
        m_entityIndex = registry->m_entityRegistry->addEntity();

        KI_DEBUG(fmt::format("ADD_ENTITY: {}", str()));

        auto* entity = registry->m_entityRegistry->getEntity(m_entityIndex);
        entity->u_materialIndex = getMaterialIndex();

        if (m_type->m_entityType == EntityType::billboard) {
            entity->u_flags |= ENTITY_BILLBOARD_BIT;
        }
        if (m_type->m_flags.noFrustum) {
            entity->u_flags |= ENTITY_NO_FRUSTUM_BIT;
        }

        entity->setObjectID(m_objectID);
    }

    if (m_controller) {
        m_controller->prepare(assets, registry, *this);
    }

    if (m_generator) {
        m_generator->prepare(assets, registry, *this);
    }
}

void Node::update(
    const RenderContext& ctx,
    Node* parent) noexcept
{
    int matrixLevel = m_matrixLevel;
    updateModelMatrix(parent);

    bool changed = false;
    if (m_controller) {
        changed = m_controller->update(ctx, *this, parent);
    }

    if (changed)
        updateModelMatrix(parent);

    if (m_camera) m_camera->update(ctx, *this);
    if (m_light) m_light->update(ctx, *this);
    if (m_generator) m_generator->update(ctx, *this, parent);

    const auto* children = ctx.m_registry->m_nodeRegistry->getChildren(*this);
    if (children) {
        for (auto& child : *children) {
            child->update(ctx, this);
        }
    }
}

void Node::updateEntity(
    const RenderContext& ctx,
    EntityRegistry* entityRegistry)
{
    if (!m_dirtyEntity || m_entityIndex == -1) return;

    auto* entity = entityRegistry->updateEntity(m_entityIndex, true);

    entity->setModelMatrix(m_modelMatrix);

    // https://stackoverflow.com/questions/27600045/the-correct-way-to-calculate-normal-matrix
    entity->setNormalMatrix(glm::mat3(glm::inverseTranspose(m_modelMatrix)));

    entity->u_materialIndex = getMaterialIndex();
    entity->u_highlightIndex = getHighlightIndex(ctx);

    entity->u_volume = getVolume();

    m_dirtyEntity = false;
}

void Node::bindBatch(const RenderContext& ctx, Batch& batch) noexcept
{
    if (m_type->m_flags.instanced) {
        // NOTE KI instanced node may not be ready, or currently not generating visible entities
        if (m_instancedIndex != -1) {
            batch.addInstanced(ctx, m_entityIndex, m_instancedIndex, m_instancedCount);
        }
    } else {
        batch.add(ctx, m_entityIndex);
    }
}

void Node::updateModelMatrix(Node* parent) noexcept
{
    const int parentMatrixLevel = parent ? parent->m_matrixLevel : -1;
    const bool dirtyParent = m_parentMatrixLevel != parentMatrixLevel;
    const bool dirtyModel = m_dirty
        || dirtyParent;
    if (!dirtyModel) return;

    const bool needPlaneNormal = m_type->m_flags.mirror && (m_dirty || dirtyParent);

    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/

    // https://learnopengl.com/Lighting/Basic-Lighting
    // http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
    // https://stackoverflow.com/questions/27600045/the-correct-way-to-calculate-normal-matrix
    // normal = mat3(transpose(inverse(model))) * aNormal;

    //const auto& translateMatrix = glm::translate(IDENTITY_MATRIX, m_position);
    //const auto& rotationMatrix = glm::toMat4(glm::quat(glm::radians(m_rotation)));
    //const auto& scaleMatrix = glm::scale(IDENTITY_MATRIX, m_scale);
    //const auto& modelMatrix = translateMatrix * rotationMatrix * scaleMatrix;

    if (parent) {
        m_modelMatrix = parent->m_modelMatrix * m_translateMatrix * m_rotationMatrix * m_scaleMatrix;
        m_parentMatrixLevel = parentMatrixLevel;
    }
    else {
        m_modelMatrix = m_translateMatrix * m_rotationMatrix * m_scaleMatrix;
    }

    m_worldPosition = m_modelMatrix[3];

    if (needPlaneNormal) {
        m_worldPlaneNormal = glm::normalize(glm::vec3(m_modelMatrix * glm::vec4(m_planeNormal, 0.f)));
    }

    m_dirtyEntity = true;
    m_matrixLevel++;
}

void Node::setPlaneNormal(const glm::vec3& planeNormal) noexcept
{
    if (m_planeNormal != planeNormal) {
        m_planeNormal = planeNormal;
        m_dirty = true;
    }
}

void Node::setPosition(const glm::vec3& pos) noexcept
{
    if (m_translateMatrix[3][0] != pos.x ||
        m_translateMatrix[3][1] != pos.y ||
        m_translateMatrix[3][2] != pos.z)
    {
        m_translateMatrix[3][0] = pos.x;
        m_translateMatrix[3][1] = pos.y;
        m_translateMatrix[3][2] = pos.z;
        m_dirty = true;
    }
}

void Node::setRotation(const glm::vec3& rotation) noexcept
{
    if (m_rotation != rotation) {
        m_rotation = rotation;
        m_rotationMatrix = glm::toMat4(glm::quat(glm::radians(m_rotation)));
        m_dirty = true;
    }
}

void Node::setScale(float scale) noexcept
{
    assert(scale >= 0);
    if (m_scaleMatrix[0][0] != scale ||
        m_scaleMatrix[1][1] != scale ||
        m_scaleMatrix[2][2] != scale)
    {
        m_scaleMatrix[0][0] = scale;
        m_scaleMatrix[1][1] = scale;
        m_scaleMatrix[2][2] = scale;
        m_dirty = true;
    }
}

void Node::setScale(const glm::vec3& scale) noexcept
{
    assert(scale.x >= 0 && scale.y >= 0 && scale.z >= 0);
    if (m_scaleMatrix[0][0] != scale.x ||
        m_scaleMatrix[1][1] != scale.y ||
        m_scaleMatrix[2][2] != scale.z)
    {
        m_scaleMatrix[0][0] = scale.x;
        m_scaleMatrix[1][1] = scale.y;
        m_scaleMatrix[2][2] = scale.z;
        m_dirty = true;
    }
}

bool Node::inFrustum(const RenderContext& ctx, float radiusFlex) const
{
    //https://en.wikibooks.org/wiki/OpenGL_Programming/Glescraft_5
    auto coords = ctx.m_matrices.u_projected * glm::vec4(m_worldPosition, 1.0);
    coords.x /= coords.w;
    coords.y /= coords.w;

    bool hit = true;
    if (coords.x < -1 || coords.x > 1 || coords.y < -1 || coords.y > 1 || coords.z < 0) {
        const auto& volume = getVolume();
        float diameter = volume.a * radiusFlex;

        if (coords.z < -diameter) {
            hit = false;
        }

        if (hit) {
            diameter /= fabsf(coords.w);
            if (fabsf(coords.x) > 1 + diameter || fabsf(coords.y > 1 + diameter)) {
                hit = false;
            }
        }
    }
    return hit;
}

void Node::setAABB(const AABB& aabb)
{
    m_aabb = aabb;
}

void Node::setTagMaterialIndex(int index)
{
    if (m_tagMaterialIndex != index) {
        m_tagMaterialIndex = index;
        m_dirtyEntity = true;
    }
}

void Node::setSelectionMaterialIndex(int index)
{
    if (m_selectionMaterialIndex != index) {
        m_selectionMaterialIndex = index;
        m_dirtyEntity = true;
    }
}

int Node::getHighlightIndex(const RenderContext& ctx) const
{
    if (ctx.assets.showHighlight) {
        if (ctx.assets.showTagged && m_tagMaterialIndex > -1) return m_tagMaterialIndex;
        if (ctx.assets.showSelection && m_selectionMaterialIndex > -1) return m_selectionMaterialIndex;
    }
    return -1;
}

int Node::lua_getId() const noexcept
{
    return m_objectID;
}

const std::string& Node::lua_getName() const noexcept
{
    return m_type->m_name;
}

int Node::lua_getCloneIndex() const noexcept
{
    return m_cloneIndex;
}

const std::array<unsigned int, 3> Node::lua_getTile() const noexcept
{
    return { m_tile.x, m_tile.y, m_tile.z };
}

const std::array<float, 3> Node::lua_getPos() const noexcept
{
    const auto& pos = getPosition();
    return { pos.x, pos.y, pos.z };
}

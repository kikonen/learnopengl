#include "Node.h"

#include <mutex>
#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "kigl/kigl.h"

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"

#include "generator/NodeGenerator.h"

#include "registry/MeshType.h"
#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/EntitySSBO.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

#include "render/Batch.h"

namespace {
    ki::node_id idBase = 100;

    std::mutex object_id_lock{};

    const static glm::mat4 IDENTITY_MATRIX{ 1.f };

    ki::node_id nextID() noexcept
    {
        std::lock_guard<std::mutex> lock(object_id_lock);
        return ++idBase;
    }
}

Node::Node(const MeshType* type)
    : m_type(type),
    m_id(nextID())
{
}

Node::~Node()
{
    KI_INFO(fmt::format("NODE: delete - {}", str()));
}

const std::string Node::str() const noexcept
{
    return fmt::format(
        "<NODE: id={}, entity={}, type={}>",
        m_id, m_transform.m_entityIndex, m_type->str());
}

void Node::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_type->getMesh()) {
        m_transform.m_entityIndex = registry->m_entityRegistry->registerEntity();
        m_transform.setMaterialIndex(m_type->getMaterialIndex());

        KI_DEBUG(fmt::format("ADD_ENTITY: {}", str()));

        ki::size_t_entity_flags flags = 0;

        if (m_type->m_entityType == EntityType::billboard) {
            flags |= ENTITY_BILLBOARD_BIT;
        }
        if (m_type->m_entityType == EntityType::sprite) {
            flags |= ENTITY_SPRITE_BIT;
            auto& shape = m_type->m_sprite.m_shapes[m_type->m_sprite.m_shapes.size() - 1];
            m_transform.m_shapeIndex = shape.m_registeredIndex;
            //m_instance.m_materialIndex = shape.m_materialIndex;
        }
        if (m_type->m_entityType == EntityType::skybox) {
            flags |= ENTITY_SKYBOX_BIT;
        }
        if (m_type->m_flags.noFrustum) {
            flags |= ENTITY_NO_FRUSTUM_BIT;
        }

        m_entityFlags = flags;
    }

    if (m_generator) {
        m_generator->prepare(assets, registry, *this);
    }
}

void Node::updateWT(
    const UpdateContext& ctx) noexcept
{
    updateModelMatrix();

    if (m_generator) m_generator->update(ctx, *this);

    if (!m_children.empty()) {
        for (auto& child : m_children) {
            child->updateWT(ctx);
        }
    }
}

void Node::snapshot() noexcept {
    if (m_transform.m_dirtySnapshot) {
        m_snapshot = m_transform;
        m_transform.m_dirtySnapshot = false;
    }

    if (m_generator) {
        m_generator->snapshot();
    }
}

bool Node::isEntity() const noexcept
{
    return m_type->getMesh() &&
        !m_type->m_flags.invisible;
}

void Node::updateEntity(
    const UpdateContext& ctx,
    EntityRegistry* entityRegistry)
{
    auto& snapshot = m_snapshot;
    if (!m_forceUpdateEntity && !snapshot.m_dirtyEntity) return;

    if (snapshot.m_entityIndex != -1)
    {
        snapshot.m_dirtyEntity |= m_forceUpdateEntity;
        auto* entity = entityRegistry->modifyEntity(snapshot.m_entityIndex, true);

        entity->u_objectID = m_id;
        entity->u_flags = m_entityFlags;
        entity->u_highlightIndex = getHighlightIndex(ctx.m_assets);

        snapshot.updateEntity(ctx, entity);
    }

    if (m_generator) {
        m_generator->updateEntity(ctx, *this, entityRegistry, m_forceUpdateEntity);
    }

    m_forceUpdateEntity = false;
}

void Node::bindBatch(const RenderContext& ctx, Batch& batch) noexcept
{
    if (m_type->m_flags.instanced) {
        if (m_instancer) {
            m_instancer->bindBatch(ctx, *this, batch);
        }
    } else {
        batch.add(ctx, m_transform.m_entityIndex);
    }
}

void Node::updateModelMatrix() noexcept
{
    auto oldLevel = m_transform.m_matrixLevel;
    if (m_parent) {
        m_transform.updateModelMatrix(m_parent->getTransform());
    }
    else {
        m_transform.updateRootMatrix();
    }
}

void Node::setTagMaterialIndex(int index)
{
    if (m_tagMaterialIndex != index) {
        m_tagMaterialIndex = index;
        m_transform.m_dirtyEntity = true;
        m_transform.m_dirtySnapshot = true;
        m_forceUpdateEntity = true;
    }
}

void Node::setSelectionMaterialIndex(int index)
{
    if (m_selectionMaterialIndex != index) {
        m_selectionMaterialIndex = index;
        m_transform.m_dirtyEntity = true;
        m_transform.m_dirtySnapshot = true;
        m_forceUpdateEntity = true;
    }
}

ki::node_id Node::lua_getId() const noexcept
{
    return m_id;
}

const std::string& Node::lua_getName() const noexcept
{
    return m_type->m_name;
}

int Node::lua_getCloneIndex() const noexcept
{
   return m_cloneIndex;
}

const std::array<float, 3> Node::lua_getPos() const noexcept
{
    const auto& pos = m_transform.getPosition();
    return { pos.x, pos.y, pos.z };
}

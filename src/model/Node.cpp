#include "Node.h"

#include <mutex>
#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "kigl/kigl.h"

#include "asset/Sprite.h"

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"

#include "generator/NodeGenerator.h"

#include "mesh/MeshType.h"

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

Node::Node(const mesh::MeshType* type)
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
    const PrepareContext& ctx)
{
    auto& registry = ctx.m_registry;

    if (m_type->getMesh()) {
        m_transform.m_entityIndex = registry->m_entityRegistry->registerEntity();
        m_transform.setMaterialIndex(m_type->getMaterialIndex());

        KI_DEBUG(fmt::format("ADD_ENTITY: {}", str()));

        ki::size_t_entity_flags flags = 0;

        if (m_type->m_entityType == mesh::EntityType::billboard) {
            flags |= ENTITY_BILLBOARD_BIT;
        }
        if (m_type->m_entityType == mesh::EntityType::sprite) {
            flags |= ENTITY_SPRITE_BIT;
            auto& shape = m_type->m_sprite->m_shapes[m_type->m_sprite->m_shapes.size() - 1];
            m_transform.m_shapeIndex = shape.m_registeredIndex;
            //m_instance.m_materialIndex = shape.m_materialIndex;
        }
        if (m_type->m_entityType == mesh::EntityType::skybox) {
            flags |= ENTITY_SKYBOX_BIT;
        }
        if (m_type->m_flags.noFrustum) {
            flags |= ENTITY_NO_FRUSTUM_BIT;
        }

        m_entityFlags = flags;
    }

    if (m_generator) {
        m_generator->prepare(ctx, *this);
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

void Node::snapshotWT() noexcept {
    auto& transform = m_transform;
    if (!m_forceUpdateSnapshot && !transform.m_dirtySnapshot) return;

    {
        assert(!transform.m_dirty);
        m_snapshotWT = transform;
        m_snapshotWT.m_dirty = true;
    }

    if (m_generator) {
        m_generator->snapshotWT(m_forceUpdateSnapshot);
    }

    transform.m_dirtySnapshot = false;
    transform.m_dirtyNormal = false;
    m_forceUpdateSnapshot = false;

    m_forceUpdateEntityRT |= m_forceUpdateEntityWT;
    m_forceUpdateEntityWT = false;
}

void Node::snapshotRT() noexcept {
    if (m_snapshotWT.m_dirty) {
        m_snapshotRT = m_snapshotWT;
        m_snapshotWT.m_dirty = false;
    }

    if (m_generator) {
        m_generator->snapshotRT(false);
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
    auto& snapshot = m_snapshotRT;
    if (!m_forceUpdateEntityRT && !snapshot.m_dirtyEntity) return;

    if (snapshot.m_entityIndex != -1)
    {
        snapshot.m_dirtyEntity |= m_forceUpdateEntityRT;
        auto* entity = entityRegistry->modifyEntity(snapshot.m_entityIndex, true);

        entity->u_objectID = m_id;
        entity->u_flags = m_entityFlags;
        entity->u_highlightIndex = getHighlightIndex(ctx.m_assets);

        snapshot.updateEntity(ctx, entity);
    }

    if (m_generator) {
        m_generator->updateEntity(ctx, *this, entityRegistry, m_forceUpdateEntityRT);
    }

    m_forceUpdateEntityRT = false;
}

void Node::updateVAO(const RenderContext& ctx) noexcept
{
    if (m_instancer) {
        m_instancer->updateVAO(ctx, *this);
    }
}

const kigl::GLVertexArray* Node::getVAO() const noexcept
{
    if (m_instancer) {
        return m_instancer->getVAO(*this);
    }
    else {
        return m_type->getVAO();
    }
}

const backend::DrawOptions& Node::getDrawOptions() const noexcept
{
    if (m_instancer) {
        return m_instancer->getDrawOptions(*this);
    }
    else {
        return m_type->getDrawOptions();
    }
}

void Node::bindBatch(const RenderContext& ctx, render::Batch& batch) noexcept
{
    if (m_instancer) {
        m_instancer->bindBatch(ctx, *this, batch);
    } else {
        batch.add(ctx, m_transform.m_entityIndex);
    }
}

void Node::updateModelMatrix() noexcept
{
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
        m_forceUpdateEntityWT = true;
        m_forceUpdateSnapshot = true;
    }
}

void Node::setSelectionMaterialIndex(int index)
{
    if (m_selectionMaterialIndex != index) {
        m_selectionMaterialIndex = index;
        m_transform.m_dirtyEntity = true;
        m_transform.m_dirtySnapshot = true;
        m_forceUpdateEntityWT = true;
        m_forceUpdateSnapshot = true;
    }
}

int Node::getHighlightIndex(const Assets& assets) const noexcept
{
    if (assets.showHighlight) {
        if (assets.showTagged && m_tagMaterialIndex > -1) return m_tagMaterialIndex;
        if (assets.showSelection && m_selectionMaterialIndex > -1) return m_selectionMaterialIndex;
    }
    return -1;
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

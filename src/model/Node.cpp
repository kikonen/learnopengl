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
        m_id, m_instance.m_entityIndex, m_type->str());
}

void Node::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_type->getMesh()) {
        m_instance.m_entityIndex = registry->m_entityRegistry->registerEntity();
        m_instance.setMaterialIndex(m_type->getMaterialIndex());

        KI_DEBUG(fmt::format("ADD_ENTITY: {}", str()));

        ki::size_t_entity_flags flags = 0;

        if (m_type->m_entityType == EntityType::billboard) {
            flags |= ENTITY_BILLBOARD_BIT;
        }
        if (m_type->m_entityType == EntityType::sprite) {
            flags |= ENTITY_SPRITE_BIT;
            auto& shape = m_type->m_sprite.m_shapes[m_type->m_sprite.m_shapes.size() - 1];
            m_instance.m_shapeIndex = shape.m_registeredIndex;
            //m_instance.m_materialIndex = shape.m_materialIndex;
        }
        if (m_type->m_entityType == EntityType::skybox) {
            flags |= ENTITY_SKYBOX_BIT;
        }
        if (m_type->m_flags.noFrustum) {
            flags |= ENTITY_NO_FRUSTUM_BIT;
        }

        m_instance.setFlags(flags);
        m_instance.setId(m_id);
    }

    if (m_generator) {
        m_generator->prepare(assets, registry, *this);
    }
}

void Node::update(
    const UpdateContext& ctx) noexcept
{
    updateModelMatrix();

    if (m_camera) m_camera->update(ctx, *this);
    if (m_light) m_light->update(ctx, *this);
    if (m_generator) m_generator->update(ctx, *this);

    const auto* children = ctx.m_registry->m_nodeRegistry->getChildren(*this);
    if (children) {
        for (auto& child : *children) {
            child->update(ctx);
        }
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
    if (m_instance.m_entityIndex != -1)
    {
        if (m_instance.m_dirtyEntity) {
            auto* entity = entityRegistry->updateEntity(m_instance.m_entityIndex, true);

            entity->u_highlightIndex = getHighlightIndex(ctx.m_assets);

            m_instance.updateEntity(ctx, entity);
        }
    }

    if (m_generator) {
        m_generator->updateEntity(ctx, *this, entityRegistry);
    }
}

void Node::bindBatch(const RenderContext& ctx, Batch& batch) noexcept
{
    if (m_type->m_flags.instanced) {
        if (m_instancer) {
            m_instancer->bindBatch(ctx, *this, batch);
        }
    } else {
        batch.add(ctx, m_instance.m_entityIndex);
    }
}

void Node::updateModelMatrix() noexcept
{
    auto oldLevel = m_instance.m_matrixLevel;
    if (m_parent) {
        m_instance.updateModelMatrix(m_parent->getInstance());
    }
    else {
        m_instance.updateRootMatrix();
    }
}

void Node::setTagMaterialIndex(int index)
{
    if (m_tagMaterialIndex != index) {
        m_tagMaterialIndex = index;
        m_instance.m_dirtyEntity = true;
    }
}

void Node::setSelectionMaterialIndex(int index)
{
    if (m_selectionMaterialIndex != index) {
        m_selectionMaterialIndex = index;
        m_instance.m_dirtyEntity = true;
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
    const auto& pos = getPosition();
    return { pos.x, pos.y, pos.z };
}

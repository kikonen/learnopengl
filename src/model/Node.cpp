#include "Node.h"

#include <mutex>
#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/GL.h"

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/EntitySSBO.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

#include "render/Batch.h"

namespace {
    int objectIDbase = 100;

    std::mutex object_id_lock{};

    const static glm::mat4 IDENTITY_MATRIX{ 1.f };

    int nextID() noexcept
    {
        std::lock_guard<std::mutex> lock(object_id_lock);
        return ++objectIDbase;
    }
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
        m_objectID, KI_UUID_STR(m_id), m_instance.m_entityIndex, m_type->str());
}

void Node::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    if (m_type->getMesh()) {
        m_instance.m_entityIndex = registry->m_entityRegistry->addEntity();
        m_instance.setMaterialIndex(getMaterialIndex());

        KI_DEBUG(fmt::format("ADD_ENTITY: {}", str()));

        int flags = 0;

        if (m_type->m_entityType == EntityType::billboard) {
            flags |= ENTITY_BILLBOARD_BIT;
        }
        if (m_type->m_flags.noFrustum) {
            flags |= ENTITY_NO_FRUSTUM_BIT;
        }

        m_instance.setFlags(flags);
        m_instance.setObjectID(m_objectID);
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

void Node::updateEntity(
    const UpdateContext& ctx,
    EntityRegistry* entityRegistry)
{
    if (m_instance.m_entityIndex != -1)
    {
        if (m_instance.m_entityDirty) {
            auto* entity = entityRegistry->updateEntity(m_instance.m_entityIndex, true);

            entity->u_highlightIndex = getHighlightIndex(ctx.m_assets);

            m_instance.updateEntity(entity);
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
    int oldLevel = m_instance.m_matrixLevel;
    if (m_parent) {
        m_instance.updateModelMatrix(m_parent->getInstance());
    }
    else {
        m_instance.updateRootMatrix();
    }

    if (oldLevel == m_instance.m_matrixLevel) return;
}

void Node::setTagMaterialIndex(int index)
{
    if (m_tagMaterialIndex != index) {
        m_tagMaterialIndex = index;
        m_instance.m_entityDirty = true;
    }
}

void Node::setSelectionMaterialIndex(int index)
{
    if (m_selectionMaterialIndex != index) {
        m_selectionMaterialIndex = index;
        m_instance.m_entityDirty = true;
    }
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

const std::array<float, 3> Node::lua_getPos() const noexcept
{
    const auto& pos = getPosition();
    return { pos.x, pos.y, pos.z };
}

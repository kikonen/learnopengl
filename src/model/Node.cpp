#include "Node.h"

#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "kigl/kigl.h"

#include "pool/IdGenerator.h"

#include "asset/Sprite.h"

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"

#include "generator/NodeGenerator.h"

#include "mesh/MeshType.h"

#include "model/EntityFlags.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/SnapshotRegistry.h"
#include "registry/EntityRegistry.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

#include "render/Batch.h"

namespace {
    IdGenerator<ki::node_id> ID_GENERATOR;

    const static glm::mat4 IDENTITY_MATRIX{ 1.f };
}

Node::Node(const mesh::MeshType* type)
    : m_type(type),
    m_id(ID_GENERATOR.nextId())
{
}

Node::~Node()
{
    KI_INFO(fmt::format("NODE: delete - {}", str()));
}

const std::string Node::str() const noexcept
{
    return fmt::format(
        "<NODE: id={}, type={}>",
        m_id, m_type->str());
}

void Node::prepare(
    const PrepareContext& ctx)
{
    auto& registry = ctx.m_registry;

    if (m_type->getMesh()) {
        KI_DEBUG(fmt::format("ADD_ENTITY: {}", str()));

        {
            m_transform.setMaterialIndex(m_type->getMaterialIndex());

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

            m_transform.m_flags = flags;
        }

    }
    m_snapshotIndex = ctx.m_registry->m_snapshotRegistry->registerSnapshot();

    if (m_generator) {
        m_generator->prepare(ctx, *this);
    }
}

void Node::updateWT(
    const UpdateContext& ctx) noexcept
{
    updateModelMatrix();

    if (m_generator) m_generator->updateWT(ctx, *this);

    if (!m_children.empty()) {
        for (auto& child : m_children) {
            child->updateWT(ctx);
        }
    }
}

bool Node::isEntity() const noexcept
{
    return m_type->getMesh() &&
        !m_type->m_flags.invisible;
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

void Node::bindBatch(
    const RenderContext& ctx,
    render::Batch& batch) noexcept
{
    if (m_instancer) {
        m_instancer->bindBatch(ctx, *this, batch);
    } else {
        const auto& snapshot = ctx.m_registry->m_snapshotRegistry->getActiveSnapshot(m_snapshotIndex);
        batch.addSnapshot(
            ctx,
            snapshot,
            m_entityIndex);
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
        m_transform.m_dirtySnapshot = true;
        if (m_generator) {
            for (auto& transform : m_generator->modifyTransforms()) {
                transform.m_dirtySnapshot = true;
            }
        }
    }
}

void Node::setSelectionMaterialIndex(int index)
{
    if (m_selectionMaterialIndex != index) {
        m_selectionMaterialIndex = index;
        m_transform.m_dirtySnapshot = true;
        if (m_generator) {
            for (auto& transform : m_generator->modifyTransforms()) {
                transform.m_dirtySnapshot = true;
            }
        }
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

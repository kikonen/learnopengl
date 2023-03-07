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
        m_objectID, KI_UUID_STR(m_id), m_instance.m_entityIndex, m_type->str());
}

void Node::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    {
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
    if (m_instance.m_entityIndex != -1)
    {
        if (m_instance.m_entityDirty) {
            auto* entity = entityRegistry->updateEntity(m_instance.m_entityIndex, true);

            entity->u_highlightIndex = getHighlightIndex(ctx.assets);

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

void Node::updateModelMatrix(Node* parent) noexcept
{
    int oldLevel = m_instance.m_matrixLevel;
    if (parent) {
        m_instance.updateModelMatrix(parent->getModelMatrix(), parent->getMatrixLevel());
    }
    else {
        m_instance.updateRootMatrix();
    }

    if (oldLevel == m_instance.m_matrixLevel) return;

    if (m_type->m_flags.mirror) {
        m_worldPlaneNormal = glm::normalize(glm::vec3(m_instance.m_modelMatrix * glm::vec4(m_planeNormal, 0.f)));
    }
}

bool Node::inFrustum(const RenderContext& ctx, float radiusFlex) const
{
    //https://en.wikibooks.org/wiki/OpenGL_Programming/Glescraft_5
    auto coords = ctx.m_matrices.u_projected * glm::vec4(getWorldPosition(), 1.0);
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

void Node::setTagMaterialIndex(int index)
{
    if (m_tagMaterialIndex != index) {
        m_tagMaterialIndex = index;
    }
}

void Node::setSelectionMaterialIndex(int index)
{
    if (m_selectionMaterialIndex != index) {
        m_selectionMaterialIndex = index;
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

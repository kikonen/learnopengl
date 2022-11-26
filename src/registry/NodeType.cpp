#include "NodeType.h"

#include <mutex>
#include <fmt/format.h>

#include "asset/Assets.h"
#include "asset/ShaderBind.h"

#include "scene/RenderContext.h"

#include "NodeRegistry.h"

namespace {
    int idBase = 0;

    std::mutex type_id_lock;

    int nextID()
    {
        std::lock_guard<std::mutex> lock(type_id_lock);
        return ++idBase;
    }
}


NodeType::NodeType()
    : typeID(nextID())
{
}

NodeType::~NodeType()
{
    KI_INFO_SB("NODE_TYPE: delete " << typeID);
}

const std::string NodeType::str() const noexcept
{
    return fmt::format("<NODE_TYPE: id={}, mesh={}>", typeID, m_mesh ? m_mesh->str() : "N/A");
}

Material* NodeType::findMaterial(std::function<bool(const Material&)> fn) noexcept
{
    if (!m_mesh) return nullptr;
    return m_mesh->findMaterial(fn);
}

void NodeType::modifyMaterials(std::function<void(Material&)> fn) noexcept
{
    if (!m_mesh) return;
    m_mesh->modifyMaterials(fn);
}

void NodeType::prepare(
    const Assets& assets,
    NodeRegistry& registry) noexcept
{
    if (!m_mesh) return;

    if (m_prepared) return;
    m_prepared = true;

    m_mesh->prepare(assets, registry);

    Shader* shader = m_nodeShader;
    if (shader) {
        shader->prepare(assets);
    }
}

void NodeType::prepareBatch(Batch& batch) noexcept
{
    if (!m_mesh) return;

    if (m_preparedBatch) return;
    m_preparedBatch = true;

    batch.prepareMesh(m_mesh->m_buffers.VAO);
}

void NodeType::bind(
    const RenderContext& ctx,
    Shader* shader) noexcept
{
    if (!m_mesh) return;

    m_boundShader = shader;

    m_mesh->bind(ctx, shader);

    if (m_flags.renderBack) {
        ctx.state.disable(GL_CULL_FACE);
    }
    else {
        ctx.state.enable(GL_CULL_FACE);
    }

    if (m_flags.wireframe) {
        ctx.state.polygonFrontAndBack(GL_LINE);
    }
}

void NodeType::unbind(const RenderContext& ctx) noexcept
{
    m_boundShader = nullptr;
    ctx.bindGlobal();
}

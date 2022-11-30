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


NodeType::NodeType(const std::string& name)
    : typeID(nextID()),
    m_name(name)
{
}

NodeType::~NodeType()
{
    KI_INFO_SB("NODE_TYPE: delete " << typeID);
}

const std::string NodeType::str() const noexcept
{
    return fmt::format(
        "<NODE_TYPE: id={}, name={}, mesh={}, buffers={}>",
        typeID, m_name, m_mesh ? m_mesh->str() : "N/A", m_buffers.str());
}

void NodeType::setMesh(std::unique_ptr<Mesh> mesh, bool umique)
{
    setMesh(mesh.get());
    m_deleter = std::move(mesh);
}

void NodeType::setMesh(Mesh* mesh)
{
    m_mesh = mesh;
    if (!m_mesh) return;

    m_materialVBO.setMaterials(m_mesh->getMaterials());
}

const Mesh* NodeType::getMesh() const
{
    return m_mesh;
}

void NodeType::modifyMaterials(std::function<void(Material&)> fn)
{
    for (auto& material : m_materialVBO.m_materials) {
        fn(material);
    }
}

void NodeType::prepare(
    const Assets& assets,
    NodeRegistry& registry) noexcept
{
    if (!m_mesh) return;

    if (m_prepared) return;
    m_prepared = true;

    m_buffers.prepare(false, false, false);
    m_materialVBO.create();

    m_mesh->prepare(assets);
    m_mesh->prepareMaterials(m_materialVBO);
    m_mesh->prepareVAO(m_buffers.VAO, m_materialVBO);

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

    batch.prepareMesh(m_buffers.VAO);
}

void NodeType::bind(
    const RenderContext& ctx,
    Shader* shader) noexcept
{
    if (!m_mesh) return;

    m_boundShader = shader;

    if (m_flags.renderBack) {
        ctx.state.disable(GL_CULL_FACE);
    }
    else {
        ctx.state.enable(GL_CULL_FACE);
    }

    if (m_flags.wireframe) {
        ctx.state.polygonFrontAndBack(GL_LINE);
    }

    glBindVertexArray(m_buffers.VAO);
}

void NodeType::unbind(const RenderContext& ctx) noexcept
{
    m_boundShader = nullptr;
    ctx.bindGlobal();
}

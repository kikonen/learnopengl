#include "registry/MeshType.h"

#include <mutex>
#include <fmt/format.h>

#include "asset/Assets.h"

#include "scene/RenderContext.h"

#include "NodeRegistry.h"
#include "MaterialRegistry.h"

namespace {
    int idBase = 0;

    std::mutex type_id_lock;

    int nextID()
    {
        std::lock_guard<std::mutex> lock(type_id_lock);
        return ++idBase;
    }
}


MeshType::MeshType(const std::string& name)
    : typeID(nextID()),
    m_name(name)
{
}

MeshType::~MeshType()
{
    KI_INFO_SB("NODE_TYPE: delete " << typeID);
}

const std::string MeshType::str() const noexcept
{
    return fmt::format(
        "<NODE_TYPE: id={}, name={}, mesh={}, vao={}>",
        typeID, m_name, m_mesh ? m_mesh->str() : "N/A", m_vao);
}

void MeshType::setMesh(std::unique_ptr<Mesh> mesh, bool umique)
{
    setMesh(mesh.get());
    m_deleter = std::move(mesh);
}

void MeshType::setMesh(Mesh* mesh)
{
    m_mesh = mesh;
    if (!m_mesh) return;

    m_materialVBO.setMaterials(m_mesh->getMaterials());
}

const Mesh* MeshType::getMesh() const
{
    return m_mesh;
}

void MeshType::modifyMaterials(std::function<void(Material&)> fn)
{
    for (auto& material : m_materialVBO.m_materials) {
        fn(material);
    }
}

void MeshType::prepare(
    const Assets& assets,
    NodeRegistry& nodeRegistry,
    MaterialRegistry& materialRegistry,
    MeshRegistry& meshRegistry)
{
    if (!m_mesh) return;

    if (m_prepared) return;
    m_prepared = true;

    m_vao.create();

    m_mesh->prepare(assets, meshRegistry);
    m_mesh->prepareMaterials(m_materialVBO);

    materialRegistry.registerMaterialVBO(m_materialVBO);

    m_drawOptions.renderBack = m_flags.renderBack;
    m_drawOptions.wireframe = m_flags.wireframe;
    m_drawOptions.materialOffset = m_materialVBO.m_offset;
    m_drawOptions.materialCount = m_materialVBO.m_entries.size();

    m_mesh->prepareVAO(m_vao, m_drawOptions);
    m_materialVBO.prepareVAO(m_vao);

    if (m_nodeShader) {
        m_nodeShader->prepare(assets);
    }
}

void MeshType::prepareBatch(Batch& batch) noexcept
{
    if (!m_mesh) return;

    if (m_preparedBatch) return;
    m_preparedBatch = true;

    batch.prepareMesh(m_vao);
}

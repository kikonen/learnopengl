#include "registry/MeshType.h"

#include <mutex>
#include <fmt/format.h>

#include "asset/Assets.h"

#include "scene/RenderContext.h"
#include "scene/BatchRegistry.h"

#include "NodeRegistry.h"
#include "MaterialRegistry.h"
#include "ModelRegistry.h"

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
    KI_INFO(fmt::format("NODE_TYPE: delete iD={}", typeID));
}

const std::string MeshType::str() const noexcept
{
    return fmt::format(
        "<NODE_TYPE: id={}, name={}, mesh={}, vao={}>",
        typeID, m_name, m_mesh ? m_mesh->str() : "N/A", m_vao ? *m_vao : -1);
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
    BatchRegistry& batchRegistry,
    NodeRegistry& nodeRegistry,
    MaterialRegistry& materialRegistry,
    ModelRegistry& modelRegistry)
{
    if (!m_mesh) return;

    if (m_prepared) return;
    m_prepared = true;

    //m_privateVAO.create();

    m_vao = m_mesh->prepare(assets, batchRegistry, modelRegistry);
    m_mesh->prepareMaterials(m_materialVBO);

    materialRegistry.registerMaterialVBO(m_materialVBO);

    m_drawOptions.renderBack = m_flags.renderBack;
    m_drawOptions.wireframe = m_flags.wireframe;
    m_drawOptions.blend = m_flags.blend;
    m_drawOptions.materialOffset = m_materialVBO.m_bufferIndex;
    m_drawOptions.singleMaterial = m_materialVBO.m_singleMaterial;

    m_mesh->prepareVAO(*m_vao, m_drawOptions);

    if (m_nodeShader) {
        m_nodeShader->prepare(assets);
    }
}

#include "registry/MeshType.h"

#include <mutex>
#include <fmt/format.h>

#include "asset/Assets.h"
#include "asset/MaterialIndex.h"
#include "asset/Mesh.h"

#include "backend/DrawOptions.h"

#include "scene/RenderContext.h"

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
        "<NODE_TYPE: id={}, name={}, mesh={}, vao={}, materialIndex={}, materialCount={}>",
        typeID, m_name, m_mesh ? m_mesh->str() : "N/A", m_vao ? *m_vao : -1, getMaterialIndex(), getMaterialCount());
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

void MeshType::modifyMaterials(std::function<void(Material&)> fn)
{
    for (auto& material : m_materialVBO.m_materials) {
        fn(material);
    }
}

int MeshType::getMaterialIndex() const
{
    auto& materialVBO = m_materialVBO;
    if (materialVBO.m_singleMaterial) {
        // NOTE KI handles "per instance" material case; per vertex separately
        return materialVBO.m_indeces.empty() ? -1 : materialVBO.m_indeces[0].m_materialIndex;
    }
    return -materialVBO.m_bufferIndex;
}

int MeshType::getMaterialCount() const
{
    return m_materialVBO.m_materials.size();
}

void MeshType::prepare(
    const Assets& assets,
    Batch& batch,
    NodeRegistry& nodeRegistry,
    MaterialRegistry& materialRegistry,
    ModelRegistry& modelRegistry)
{
    if (!m_mesh) return;

    if (m_prepared) return;
    m_prepared = true;

    //m_privateVAO.create();

    m_vao = m_mesh->prepare(assets, batch, modelRegistry);
    m_mesh->prepareMaterials(m_materialVBO);

    materialRegistry.registerMaterialVBO(m_materialVBO);

    m_drawOptions.renderBack = m_flags.renderBack;
    m_drawOptions.wireframe = m_flags.wireframe;
    m_drawOptions.blend = m_flags.blend;
    m_drawOptions.instanced = m_flags.instanced;

    m_mesh->prepareVAO(*m_vao, m_drawOptions);

    if (m_nodeShader) {
        m_nodeShader->prepare(assets);
    }
}

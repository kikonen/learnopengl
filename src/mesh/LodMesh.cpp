#include "LodMesh.h"

#include <fmt/format.h>

#include "Mesh.h"
#include "MaterialSet.h"

#include "registry/MaterialRegistry.h"

namespace mesh {
    LodMesh::LodMesh()
    {}

    LodMesh::LodMesh(Mesh* mesh)
    {
        setMesh(mesh);
    }

    LodMesh::LodMesh(LodMesh&& o) noexcept
    {
        m_mesh = o.m_mesh;
        m_deleter = std::move(o.m_deleter);
        m_materialSet = std::move(o.m_materialSet);

        o.m_mesh = nullptr;
    }

    LodMesh::~LodMesh()
    {}

    LodMesh& LodMesh::operator=(LodMesh&& o) noexcept
    {
        m_mesh = o.m_mesh;
        m_deleter = std::move(o.m_deleter);
        m_materialSet = std::move(o.m_materialSet);

        o.m_mesh = nullptr;

        return *this;
    }

    std::string LodMesh::str() const noexcept
    {
        return fmt::format(
            "<MESH_LOD: mesh={}, materialIndex={}, materialCount={}>",
            m_mesh ? m_mesh->str() : "N/A",
            m_materialSet.getMaterialIndex(),
            m_materialSet.getMaterialCount());
    }

    void LodMesh::setMesh(
        std::unique_ptr<Mesh> mesh,
        bool umique) noexcept
    {
        setMesh(mesh.get());
        m_deleter = std::move(mesh);
    }

    void LodMesh::setMesh(Mesh* mesh) noexcept
    {
        m_mesh = mesh;
        if (!m_mesh) return;
    }

    void LodMesh::setupMeshMaterials(
        const Material& defaultMaterial,
        bool useDefaultMaterial,
        bool forceDefaultMaterial)
    {
        m_materialSet.setDefaultMaterial(defaultMaterial, useDefaultMaterial, forceDefaultMaterial);
        if (m_mesh) {
            m_materialSet.setMaterials(m_mesh->getMaterials());
        }
    }

    void LodMesh::registerMaterials()
    {
        // NOTE KI registered *first*, so that mesh has access to registeredMaterialIndex
        for (auto& material : m_materialSet.modifyMaterials()) {
            MaterialRegistry::get().registerMaterial(material);
        }

        // apply registeredMaterialIndex
        if (m_mesh) {
            m_mesh->prepareMaterials(m_materialSet);
        }

        MaterialRegistry::get().registerVertexMaterials(m_materialSet);
    }

    void LodMesh::prepareRT(const PrepareContext& ctx)
    {
        if (m_mesh) {
            m_vao = m_mesh->prepareRT(ctx);
        }
    }
}

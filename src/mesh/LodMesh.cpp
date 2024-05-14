#include "LodMesh.h"

#include <fmt/format.h>

#include "asset/Material.h"

#include "Mesh.h"

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
        m_material = o.m_material;
        m_lod = o.m_lod;
        m_deleter = std::move(o.m_deleter);
        m_vao = o.m_vao;

        o.m_mesh = nullptr;
    }

    LodMesh::~LodMesh()
    {}

    LodMesh& LodMesh::operator=(LodMesh&& o) noexcept
    {
        m_mesh = o.m_mesh;
        m_material = o.m_material;
        m_lod = o.m_lod;
        m_deleter = std::move(o.m_deleter);
        m_vao = o.m_vao;

        o.m_mesh = nullptr;

        return *this;
    }

    std::string LodMesh::str() const noexcept
    {
        return fmt::format(
            "<MESH_LOD: mesh={}, materialIndex={}>",
            m_mesh ? m_mesh->str() : "N/A",
            m_lod.m_materialIndex);
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

        // NOTE KI copy of material for isntance
        // => material *is* per mesh type
        // => Sharing *might* be sometims possible, in practice tricky
        m_material = mesh->getMaterial();
    }

    void LodMesh::registerMaterials()
    {
        if (!m_mesh) return;

        MaterialRegistry::get().registerMaterial(m_material);

        m_lod.m_materialIndex = m_material.m_registeredIndex;
    }

    void LodMesh::prepareRT(const PrepareContext& ctx)
    {
        if (m_mesh) {
            m_vao = m_mesh->prepareRT(ctx);
            m_mesh->prepareLod(*this);
        }
    }
}

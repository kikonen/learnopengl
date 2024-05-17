#include "LodMesh.h"

#include <regex>

#include <fmt/format.h>

#include "asset/Assets.h"
#include "asset/Material.h"

#include "util/Util.h"

#include "Mesh.h"

#include "registry/MaterialRegistry.h"

namespace {
    const std::vector<std::regex> lodMatchers{
        std::regex(".*lod[0-9]+"),
    };

    int16_t resolveLodLevel(const std::string& name)
    {
        const auto& str = util::toLower(name);
        if (!util::matchAny(lodMatchers, str)) return -1;

        std::string output = std::regex_replace(
            str,
            std::regex(".*lod([0-9]+)"),
            std::string("$1")
        );

        return stoi(output);
    }
}

namespace mesh {
    LodMesh::LodMesh()
    {}

    LodMesh::LodMesh(Mesh* mesh)
    {
        setMesh(mesh);
    }

    LodMesh::LodMesh(LodMesh&& o) noexcept
    {
        m_lodLevel = o.m_lodLevel;
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
        m_lodLevel = o.m_lodLevel;
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
            "<LOD_MESH: level={}, mesh={}, materialIndex={}>",
            m_lodLevel,
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

        auto level = resolveLodLevel(mesh->m_name);
        if (level == -1) {
            level = resolveLodLevel(mesh->m_nodeName);
        }

        if (level >= 0) {
            m_lodLevel = level;
            setDistance((m_lodLevel + 1) * 20.f);
        }

        // NOTE KI copy of material for isntance
        // => material *is* per mesh type
        // => Sharing *might* be sometims possible, in practice tricky
        m_material = mesh->getMaterial();
    }

    void LodMesh::registerMaterials()
    {
        if (!m_mesh) return;

        const auto& assets = Assets::get();

        if (assets.useLodDebug) {
            if (m_lodLevel > 0)
                m_material.kd = glm::vec4{ 0.5f, 0.f, 0.f, 1.f };
            if (m_lodLevel > 1)
                m_material.kd = glm::vec4{ 0.f, 0.5f, 0.f, 1.f };
            if (m_lodLevel > 2)
                m_material.kd = glm::vec4{ 0.f, 0.f, 0.5f, 1.f };
        }

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

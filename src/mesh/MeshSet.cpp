#include "MeshSet.h"

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/Log.h"
#include "util/Util.h"

#include "animation/RigContainer.h"

#include "mesh/ModelMesh.h"

namespace {
    std::string extractName(std::string_view meshPath) {
        auto filePath = util::joinPath("", meshPath);
        return util::baseName(filePath);
    }
}

namespace mesh {
    MeshSet::MeshSet(
        std::string_view rootDir,
        std::string_view path)
        : m_rootDir{ rootDir },
        m_path{ path },
        m_name{ extractName(path) }

    {}

    MeshSet::~MeshSet() = default;

    std::string MeshSet::str() const noexcept
    {
        return fmt::format(
            "<MESH_SET: rootDir={}, meshPath={}, name={}>",
            m_rootDir, m_path, m_name);
    }

    bool MeshSet::isEmpty() const noexcept
    {
        return m_meshes.empty();
    }

    bool MeshSet::isRigged() const noexcept
    {
        return m_rig->hasBones();
    }

    void MeshSet::prepareVolume() noexcept {
        m_aabb = calculateAABB();
    }

    AABB MeshSet::calculateAABB() const noexcept
    {
        AABB aabb{ true };

        for (auto& mesh : m_meshes) {
            mesh->prepareVolume();
            aabb.merge(mesh->getAABB());
        }

        return aabb;
    }

    mesh::Mesh* MeshSet::addMesh(std::unique_ptr<mesh::Mesh>&& mesh) noexcept
    {
        m_meshes.push_back(std::move(mesh));
        return m_meshes[m_meshes.size() - 1].get();
    }

    std::vector<std::unique_ptr<mesh::Mesh>>& MeshSet::getMeshes() noexcept
    {
        return m_meshes;
    }
}

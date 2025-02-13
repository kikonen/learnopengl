#include "MeshSet.h"

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/Log.h"
#include "util/util.h"
#include "util/file.h"

#include "animation/RigContainer.h"

#include "mesh/Mesh.h"

namespace {
    std::string extractName(std::string_view meshPath) {
        auto filePath = util::joinPath("", meshPath);
        KI_INFO_OUT(fmt::format("EXTRACT_NAME: path={}, mesh_set={}",
            std::string{ filePath },
            util::stem(filePath)));
        return util::stem(filePath);
    }
}

namespace mesh {
    MeshSet::MeshSet(
        std::string_view rootDir,
        std::string_view path,
        bool smoothNormals,
        bool forceNormals)
        : m_rootDir{ rootDir },
        m_path{ path },
        m_dir{ util::dirName(path) },
        m_name{ extractName(path) },
        m_smoothNormals{ smoothNormals },
        m_forceNormals{ forceNormals }
    {}

    MeshSet::~MeshSet() = default;

    std::string MeshSet::str() const noexcept
    {
        return fmt::format(
            "<MESH_SET: meshPath={}, name={}>",
            m_path, m_name);
    }

    bool MeshSet::isEmpty() const noexcept
    {
        return m_meshes.empty();
    }

    bool MeshSet::isRigged() const noexcept
    {
        return !!m_rig;
    }

    AABB MeshSet::calculateAABB(const glm::mat4& transform) const noexcept
    {
        if (m_meshes.empty()) return {};

        AABB aabb{ true };

        for (auto& mesh : m_meshes) {
            aabb.minmax(mesh->calculateAABB(transform));
        }

        aabb.updateVolume();

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

    std::string MeshSet::getSummary()
    {
        std::string sb;
        sb.reserve(10000);

        auto appendLine = [](auto& sb, auto level, const auto& line) {
            for (int i = 0; i < level; i++) {
                sb += "    ";
            }
            sb += line;
            sb += "\n";
        };

        {
            const auto& line = fmt::format(
                "MESH_SET: {}",
                m_name);
            appendLine(sb, 0, line);
        }

        int index = 0;

        for (const auto& mesh : m_meshes) {
            {
                const auto& line = fmt::format(
                    "[{}] {}",
                    index,
                    mesh->str());

                appendLine(sb, 0, line);
            }
            {
                const auto* material = mesh->getMaterial();

                const auto& line = fmt::format(
                    "    {}",
                    material ? material->str() : "NULL");

                appendLine(sb, 0, line);
            }

            index++;
        }

        return sb;
    }
}

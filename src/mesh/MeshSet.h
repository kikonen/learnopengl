#pragma once

#include <string>
#include <vector>
#include <memory>

#include "util/Ref.h"

#include "asset/AABB.h"

#include "animation/AnimationPath.h"


namespace animation
{
}

namespace mesh_set
{
    class AssimpImporter;
}

namespace mesh
{
    class Mesh;
    class MeshImporter;

    class MeshSet final : public util::RefCountedSimple
    {
        friend class mesh_set::AssimpImporter;
        friend class MeshImporter;

    public:
        MeshSet(
            std::string_view id,
            std::string_view rootDir,
            std::string_view path,
            bool smoothNormals,
            bool forceNormals,
            const std::vector<animation::AnimationPath>& animationPaths);

        ~MeshSet();

        std::string str() const noexcept;

        bool empty() const noexcept;
        bool isRigged() const noexcept;

        // Take ownership of mesh
        mesh::Mesh* addMesh(
            const util::Ref<mesh::Mesh>& mesh) noexcept;

        const std::vector<util::Ref<mesh::Mesh>>& getMeshes() const noexcept;

        std::vector<util::Ref<mesh::Mesh>>& modifyMeshes() noexcept;

        template<typename T>
        inline const T* getMesh(size_t index) const noexcept
        {
            return dynamic_cast<const T*>(m_meshes[index].get());
        }

        AABB calculateAABB(const glm::mat4& transform) const noexcept;

        std::string getSummary() const;

    public:
        const std::string m_id;
        const std::string m_rootDir;
        const std::string m_dir;
        const std::string m_path;
        const std::string m_name;

        const bool m_smoothNormals{ false };
        const bool m_forceNormals{ false };

        const std::vector<animation::AnimationPath> m_animationPaths;

        std::string m_filePath;

    private:
        std::vector<util::Ref<mesh::Mesh>> m_meshes;
    };
}

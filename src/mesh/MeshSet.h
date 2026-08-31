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

    namespace decoder
    {
        class MeshSetDecoder;
    }
}

namespace mesh
{
    class Mesh;
    class MeshImporter;

    class MeshSet final : public util::RefCountedSimple
    {
        friend class mesh_set::AssimpImporter;
        friend class mesh_set::decoder::MeshSetDecoder;
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

        const std::string& getId() const noexcept
        {
            return m_id;
        }

        const std::string& getRootDir() const noexcept
        {
            return m_rootDir;
        }

        const std::string& getDir() const noexcept
        {
            return m_dir;
        }

        const std::string& getPath() const noexcept
        {
            return m_path;
        }

        const std::string& getName() const noexcept
        {
            return m_name;
        }

        bool getSmoothNormals() const noexcept
        {
            return m_smoothNormals;
        }

        bool getForceNormals() const noexcept
        {
            return m_forceNormals;
        }

        const std::string& getFilePath() const noexcept
        {
            return m_filePath;
        }

        const std::vector<animation::AnimationPath>& getAnimationPaths() const noexcept
        {
            return m_animationPaths;
        }

    private:
        // immutable
        std::string m_id;
        // immutable
        std::string m_rootDir;
        // immutable
        std::string m_dir;
        // immutable
        std::string m_path;
        // immutable
        std::string m_name;

        // immutable
        bool m_smoothNormals{ false };
        // immutable
        bool m_forceNormals{ false };

        // immutable
        std::vector<animation::AnimationPath> m_animationPaths;

        std::string m_filePath;

        std::vector<util::Ref<mesh::Mesh>> m_meshes;
    };
}

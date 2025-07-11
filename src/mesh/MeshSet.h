#pragma once

#include <string>
#include <vector>
#include <memory>

#include "asset/AABB.h"

namespace animation {
    struct RigContainer;
}

namespace mesh {
    class Mesh;
    class ModelLoader;
    class AssimpLoader;

    class MeshSet {
        friend class MeshLoader;
        friend class AssimpLoader;

    public:
        MeshSet(
            std::string_view path,
            std::string_view rootDir,
            bool smoothNormals,
            bool forceNormals);

        ~MeshSet();

        std::string str() const noexcept;

        bool isEmpty() const noexcept;
        bool isRigged() const noexcept;

        // Take ownership of mesh
        mesh::Mesh* addMesh(std::shared_ptr<mesh::Mesh>&& mesh) noexcept;

        const std::vector<std::shared_ptr<mesh::Mesh>>& getMeshes() const noexcept;

        template<typename T>
        inline const T* getMesh(size_t index) const noexcept
        {
            return dynamic_cast<T*>(m_meshes[index].get());
        }

        AABB calculateAABB(const glm::mat4& transform) const noexcept;

        std::string getSummary() const;

    public:
        const std::string m_rootDir;
        const std::string m_dir;
        const std::string m_path;
        const std::string m_name;

        const bool m_smoothNormals{ false };
        const bool m_forceNormals{ false };

        std::string m_filePath;

        // NOTE KI rig is set only for rigged (with bones) case
        std::shared_ptr<animation::RigContainer> m_rig;

    private:
        std::vector<std::shared_ptr<mesh::Mesh>> m_meshes;
    };
}

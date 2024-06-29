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
            std::string_view rootDir);
        ~MeshSet();

        std::string str() const noexcept;

        bool isEmpty() const noexcept;
        bool isRigged() const noexcept;

        const AABB& getAABB() const noexcept
        {
            return m_aabb;
        }

        // Take ownership of mesh
        mesh::Mesh* addMesh(std::unique_ptr<mesh::Mesh>&& mesh) noexcept;

        std::vector<std::unique_ptr<mesh::Mesh>>& getMeshes() noexcept;

        void prepareVolume() noexcept;

    private:
        AABB calculateAABB() const noexcept;

    public:
        const std::string m_rootDir;
        const std::string m_path;
        const std::string m_name;

        std::string m_filePath;

        // NOTE KI rig is set only for rigged (with bones) case
        std::shared_ptr<animation::RigContainer> m_rig;

    private:
        AABB m_aabb;
        std::vector<std::unique_ptr<mesh::Mesh>> m_meshes;
    };
}

#pragma once

#include <string>
#include <memory>
#include <atomic>

#include "glm/glm.hpp"

#include "material/Material.h"

namespace mesh
{
    class MeshSet;
}

namespace mesh_set
{
    class MeshSetLoader
    {
    public:
        MeshSetLoader(
            const std::shared_ptr<std::atomic_bool>& alive);

        virtual ~MeshSetLoader();

        // @return true if
        bool load(
            mesh::MeshSet& meshSet,
            Material* defaultMaterial,
            bool forceDefaultMaterial);

    protected:
        virtual void loadData(
            mesh::MeshSet& meshSet) = 0;

    protected:
        Material m_defaultMaterial;
        bool m_forceDefaultMaterial{ false };

        std::shared_ptr<std::atomic_bool> m_alive;
    };
}

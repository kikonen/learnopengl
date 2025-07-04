#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "pool/NodeHandle.h"

#include "InputGeom.h"

namespace mesh
{
    struct MeshInstance;
}

namespace nav
{
    class InputGeom;

    class InputCollection
    {
    public:
        InputCollection();
        ~InputCollection();

        void prepareBuild(InputCollection& o);

        void addNode(pool::NodeHandle nodeHandle);
        void removeNode(pool::NodeHandle nodeHandle);

        void clearMeshInstances();
        void addMeshInstance(const mesh::MeshInstance& meshInstance);

        const std::vector<std::unique_ptr<InputGeom>>& getGeometries() const
        {
            return m_geometries;
        }

        bool empty() const { return m_geometries.empty(); }
        bool dirty() const { return m_dirty; }

        void build();

        const glm::vec3& getNavMeshBoundsMin() const { return m_navMeshBMin; }
        const glm::vec3& getNavMeshBoundsMax() const { return m_navMeshBMax; }

        int getMaxTriCount() const { return m_maxTriCount; }

    private:
        bool m_dirty{ true };

        std::vector<pool::NodeHandle> m_nodeHandles;
        std::unique_ptr<std::vector<mesh::MeshInstance>> m_meshInstances;

        std::vector<std::unique_ptr<nav::InputGeom>> m_geometries;

        glm::vec3 m_navMeshBMin{ 0.f };
        glm::vec3 m_navMeshBMax{ 0.f };

        int m_maxTriCount{ 0 };
    };
}

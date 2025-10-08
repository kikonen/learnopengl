#include "InputCollection.h"

#include "util/glm_util.h"

#include "model/Node.h"

#include "mesh/MeshInstance.h"
#include "mesh/LodMesh.h"

#include "InputGeom.h"

namespace nav
{
    InputCollection::InputCollection()
        : m_meshInstances{ std::make_unique<std::vector<mesh::MeshInstance>>() }
    { }

    InputCollection::~InputCollection()
    {
    }

    void InputCollection::prepareBuild(InputCollection& o)
    {
        m_dirty = o.m_dirty;
        m_nodeHandles = o.m_nodeHandles;
        m_meshInstances = std::make_unique<std::vector<mesh::MeshInstance>>(*o.m_meshInstances);

        m_geometries.clear();
        m_navMeshBMin = glm::vec3{ 0.f };
        m_navMeshBMax = glm::vec3{ 0.f };
        m_maxTriCount = 0;

        o.m_dirty = false;
    }

    void InputCollection::addNode(pool::NodeHandle nodeHandle)
    {
        m_dirty = true;
        m_nodeHandles.push_back(nodeHandle);
    }

    void InputCollection::removeNode(pool::NodeHandle nodeHandle)
    {
        if (nodeHandle.removeFrom(m_nodeHandles)) {
            m_dirty = true;
        }
    }

    void InputCollection::clearMeshInstances()
    {
        if (m_meshInstances->empty()) return;

        m_dirty = true;
        m_meshInstances->clear();
    }

    void InputCollection::addMeshInstance(const mesh::MeshInstance& meshInstance)
    {
        m_dirty = true;
        m_meshInstances->push_back(meshInstance);
    }

    void InputCollection::build(const std::atomic_bool& alive)
    {
        if (!m_dirty) return;

        m_maxTriCount = 0;
        m_geometries.clear();

        for (auto& nodeHandle : m_nodeHandles) {
            if (!alive) break;

            auto* node = nodeHandle.toNode();
            if (!node) continue;

            const auto& state = node->getState();

            for (const auto& lodMesh : node->getLodMeshes()) {
                auto geom = std::make_unique<nav::InputGeom>(
                    state.getModelMatrix() *
                    lodMesh.m_baseTransform,
                    lodMesh.m_mesh);
                m_geometries.push_back(std::move(geom));
            }
        }

        for (const auto& meshInstance : *m_meshInstances) {
            if (!alive) break;

            auto geom = std::make_unique<nav::InputGeom>(
                meshInstance.getTransform(),
                meshInstance.m_mesh);
                m_geometries.push_back(std::move(geom));
        }

        for (auto& geom : m_geometries) {
            if (!alive) break;

            geom->build();
            m_maxTriCount = std::max(m_maxTriCount, geom->getTriCount());
        }

        {
            bool first = true;
            glm::vec3 navMin{ 0.f };
            glm::vec3 navMax{ 0.f };
            for (auto& geom : m_geometries) {
                const auto& geomMin = geom->getMeshBoundsMin();
                const auto& geomMax = geom->getMeshBoundsMax();

                if (first) {
                    navMin = geomMin;
                    navMax = geomMax;
                    first = false;
                }

                util::minmax(geomMin, navMin, navMax);
                util::minmax(geomMax, navMin, navMax);
            }
            m_navMeshBMin = navMin;
            m_navMeshBMax = navMax;
        }

        m_dirty = false;
    }
}

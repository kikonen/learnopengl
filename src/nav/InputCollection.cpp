#include "InputCollection.h"

#include "util/glm_util.h"

#include "model/Node.h"

#include "mesh/MeshInstance.h"
#include "mesh/MeshType.h"
#include "mesh/LodMesh.h"

#include "InputGeom.h"

namespace nav
{
    InputCollection::InputCollection()
        : m_meshInstances{ std::make_unique<std::vector<mesh::MeshInstance>>() }
    { }

    InputCollection::~InputCollection()
    { }

    void InputCollection::addNode(pool::NodeHandle nodeHandle)
    {
        m_dirty = true;
        m_nodeHandles.push_back(nodeHandle);
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

    void InputCollection::build()
    {
        if (!m_dirty) return;

        m_maxTriCount = 0;
        m_geometries.clear();

        for (auto& nodeHandle : m_nodeHandles) {
            auto* node = nodeHandle.toNode();
            if (!node) continue;

            const auto& state = node->getState();

            for (const auto& lodMesh : node->getType()->getLodMeshes()) {
                auto geom = std::make_unique<nav::InputGeom>(
                    state.getModelMatrix() *
                    lodMesh.m_transform,
                    lodMesh.m_mesh);
                m_geometries.push_back(std::move(geom));
            }
        }

        for (const auto& meshInstance : *m_meshInstances) {
            auto geom = std::make_unique<nav::InputGeom>(
                meshInstance.getTransform(),
                meshInstance.m_mesh);
                m_geometries.push_back(std::move(geom));
        }

        for (auto& geom : m_geometries) {
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

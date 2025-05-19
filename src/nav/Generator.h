#pragma once

#include <vector>
#include <memory>

#include <recastnavigation/Recast.h>

#include "pool/NodeHandle.h"

#include "InputCollection.h"
#include "BuildSettings.h"

namespace mesh
{
    struct MeshInstance;
}

namespace nav
{
    class RecastContainer;
    class BuildContext;
    class InputGeom;

    class Generator
    {
    public:
        Generator(std::shared_ptr<nav::RecastContainer> container);
        ~Generator();

        void clear();
        void cleanup();

        void registerNode(pool::NodeHandle nodeHandle);

        void clearMeshInstances();
        void registerMeshInstance(const mesh::MeshInstance& meshInstance);

        // Build must be done after registering all meshes
        bool build();

    public:
        BuildSettings m_settings;
        std::unique_ptr<BuildContext> m_ctx;

        bool m_filterLowHangingObstacles;
        bool m_filterLedgeSpans;
        bool m_filterWalkableLowHeightSpans;

        bool m_keepInterResults;

    private:
        std::shared_ptr<nav::RecastContainer> m_container;

        nav::InputCollection m_inputCollection;

        rcConfig m_cfg{};

        unsigned char* m_triareas{ nullptr };
        rcHeightfield* m_solid{ nullptr };
        rcCompactHeightfield* m_chf{ nullptr };
        rcContourSet* m_cset{ nullptr };
        rcPolyMesh* m_pmesh{ nullptr };
        rcPolyMeshDetail* m_dmesh{ nullptr };
    };
}

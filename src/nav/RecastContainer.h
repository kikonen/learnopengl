#pragma once

#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/DetourNavMeshQuery.h>
#include <recastnavigation/DetourCrowd.h>

namespace nav
{
    // Shared data between generator and resolver
    class RecastContainer
    {
    public:
        RecastContainer();
        ~RecastContainer();

        void clear();
        void prepare();
        void cleanup();

    public:
        class dtNavMesh* m_navMesh{ nullptr };
        class dtNavMeshQuery* m_navQuery{ nullptr };
        class dtCrowd* m_crowd{ nullptr };
    };
}

#include "RecastContainer.h"

#include <recastnavigation/Recast.h>
#include <recastnavigation/DetourCrowd.h>

namespace nav
{
    RecastContainer::RecastContainer()
    {
        m_navQuery = dtAllocNavMeshQuery();
        m_crowd = dtAllocCrowd();
    }

    RecastContainer::~RecastContainer()
    {
        dtFreeNavMeshQuery(m_navQuery);
        dtFreeNavMesh(m_navMesh);
        dtFreeCrowd(m_crowd);
    }

    void RecastContainer::cleanup()
    {
        dtFreeNavMesh(m_navMesh);
        m_navMesh = nullptr;
    }
}

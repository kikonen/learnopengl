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
        clear();
    }

    void RecastContainer::clear()
    {
        dtFreeNavMeshQuery(m_navQuery);
        m_navQuery = nullptr;

        dtFreeNavMesh(m_navMesh);
        m_navMesh = nullptr;

        dtFreeCrowd(m_crowd);
        m_crowd = nullptr;
    }

    void RecastContainer::cleanup()
    {
        dtFreeNavMesh(m_navMesh);
        m_navMesh = nullptr;
    }
}

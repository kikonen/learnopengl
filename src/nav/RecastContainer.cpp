#include "RecastContainer.h"

#include <recastnavigation/Recast.h>
#include <recastnavigation/DetourCrowd.h>

namespace nav
{
    RecastContainer::RecastContainer()
    {
    }

    RecastContainer::~RecastContainer()
    {
        clear();
    }

    void RecastContainer::clear()
    {
        cleanup();

        dtFreeNavMeshQuery(m_navQuery);
        m_navQuery = nullptr;

        dtFreeCrowd(m_crowd);
        m_crowd = nullptr;
    }

    void RecastContainer::prepare()
    {
        clear();

        m_navQuery = dtAllocNavMeshQuery();
        m_crowd = dtAllocCrowd();
    }

    void RecastContainer::cleanup()
    {
        dtFreeNavMesh(m_navMesh);
        m_navMesh = nullptr;
    }
}

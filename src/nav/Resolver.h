#pragma once

#include <memory>

#include <recastnavigation/Recast.h>
#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/DetourNavMeshQuery.h>

namespace nav
{
    class RecastContainer;
    struct Query;
    struct Path;

    class Resolver
    {
    public:
        Resolver(std::shared_ptr<nav::RecastContainer> container);
        ~Resolver();

        void clear();

        Path resolve(const Query& query);

    private:
        std::shared_ptr<nav::RecastContainer> m_container;

        static const int MAX_POLYS = 256;
        static const int MAX_SMOOTH = 2048;

        float m_polyPickExt[3];
        dtQueryFilter m_filter;

        dtPolyRef m_startRef{ 0 };
        dtPolyRef m_endRef{ 0 };

        dtPolyRef m_polys[MAX_POLYS];
        int m_polysCount{ 0 };

        dtPolyRef m_polyPath[MAX_POLYS];

        float m_smoothPath[MAX_SMOOTH * 3];
        int m_smoothPathCount{ 0 };
    };
}

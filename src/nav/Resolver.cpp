#include "Resolver.h"

#include <recastnavigation/DetourCommon.h>
#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/DetourNavMeshQuery.h>
#include <recastnavigation/DetourPathCorridor.h>


#include "RecastContainer.h"
#include "Query.h"
#include "Path.h"

namespace
{
    static void getPolyCenter(dtNavMesh* navMesh, dtPolyRef ref, float* center)
    {
        center[0] = 0;
        center[1] = 0;
        center[2] = 0;

        const dtMeshTile* tile = 0;
        const dtPoly* poly = 0;
        dtStatus status = navMesh->getTileAndPolyByRef(ref, &tile, &poly);
        if (dtStatusFailed(status))
            return;

        for (int i = 0; i < (int)poly->vertCount; ++i)
        {
            const float* v = &tile->verts[poly->verts[i] * 3];
            center[0] += v[0];
            center[1] += v[1];
            center[2] += v[2];
        }
        const float s = 1.0f / poly->vertCount;
        center[0] *= s;
        center[1] *= s;
        center[2] *= s;
    }

    inline bool inRange(const float* v1, const float* v2, const float r, const float h)
    {
        const float dx = v2[0] - v1[0];
        const float dy = v2[1] - v1[1];
        const float dz = v2[2] - v1[2];
        return (dx * dx + dz * dz) < r * r && fabsf(dy) < h;
    }

    // This function checks if the path has a small U-turn, that is,
    // a polygon further in the path is adjacent to the first polygon
    // in the path. If that happens, a shortcut is taken.
    // This can happen if the target (T) location is at tile boundary,
    // and we're (S) approaching it parallel to the tile edge.
    // The choice at the vertex can be arbitrary,
    //  +---+---+
    //  |:::|:::|
    //  +-S-+-T-+
    //  |:::|   | <-- the step can end up in here, resulting U-turn path.
    //  +---+---+
    static int fixupShortcuts(
        dtPolyRef* path,
        int npath,
        const dtNavMeshQuery* navQuery)
    {
        if (npath < 3)
            return npath;

        // Get connected polygons
        static const int maxNeis = 16;
        dtPolyRef neis[maxNeis];
        int nneis = 0;

        const dtMeshTile* tile = 0;
        const dtPoly* poly = 0;
        if (dtStatusFailed(navQuery->getAttachedNavMesh()->getTileAndPolyByRef(path[0], &tile, &poly)))
            return npath;

        for (unsigned int k = poly->firstLink; k != DT_NULL_LINK; k = tile->links[k].next)
        {
            const dtLink* link = &tile->links[k];
            if (link->ref != 0)
            {
                if (nneis < maxNeis)
                    neis[nneis++] = link->ref;
            }
        }

        // If any of the neighbour polygons is within the next few polygons
        // in the path, short cut to that polygon directly.
        static const int maxLookAhead = 6;
        int cut = 0;
        for (int i = dtMin(maxLookAhead, npath) - 1; i > 1 && cut == 0; i--) {
            for (int j = 0; j < nneis; j++)
            {
                if (path[i] == neis[j]) {
                    cut = i;
                    break;
                }
            }
        }
        if (cut > 1)
        {
            int offset = cut - 1;
            npath -= offset;
            for (int i = 1; i < npath; i++)
                path[i] = path[i + offset];
        }

        return npath;
    }

    static bool getSteerTarget(
        const dtNavMeshQuery* navQuery,
        const float* startPos,
        const float* endPos,
        const float minTargetDist,
        const dtPolyRef* path,
        const int pathSize,
        float* steerPos,
        unsigned char& steerPosFlag,
        dtPolyRef& steerPosRef,
        float* outPoints = 0,
        int* outPointCount = 0)
    {
        // Find steer target.
        static const int MAX_STEER_POINTS = 3;
        float steerPath[MAX_STEER_POINTS * 3];
        unsigned char steerPathFlags[MAX_STEER_POINTS];
        dtPolyRef steerPathPolys[MAX_STEER_POINTS];
        int nsteerPath = 0;
        navQuery->findStraightPath(startPos, endPos, path, pathSize,
            steerPath, steerPathFlags, steerPathPolys, &nsteerPath, MAX_STEER_POINTS);
        if (!nsteerPath)
            return false;

        if (outPoints && outPointCount)
        {
            *outPointCount = nsteerPath;
            for (int i = 0; i < nsteerPath; ++i)
                dtVcopy(&outPoints[i * 3], &steerPath[i * 3]);
        }


        // Find vertex far enough to steer to.
        int ns = 0;
        while (ns < nsteerPath)
        {
            // Stop at Off-Mesh link or when point is further than slop away.
            if ((steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ||
                !inRange(&steerPath[ns * 3], startPos, minTargetDist, 1000.0f))
                break;
            ns++;
        }
        // Failed to find good point to steer to.
        if (ns >= nsteerPath)
            return false;

        dtVcopy(steerPos, &steerPath[ns * 3]);
        steerPos[1] = startPos[1];
        steerPosFlag = steerPathFlags[ns];
        steerPosRef = steerPathPolys[ns];

        return true;
    }
}

namespace nav
{
    Resolver::Resolver(std::shared_ptr<nav::RecastContainer> container)
        : m_container{ container }
    {
        m_polyPickExt[0] = 2;
        m_polyPickExt[1] = 4;
        m_polyPickExt[2] = 2;
    }

    Resolver::~Resolver() = default;

    Path Resolver::resolve(const Query& query)
    {
        const dtNavMesh* navMesh = m_container->m_navMesh;
        const dtNavMeshQuery* navQuery = m_container->m_navQuery;

        if (!navMesh || !navQuery) return {};

        // TODO KI query to recast query

        const float* startPos = &query.m_startPos[0];
        const float* endPos = &query.m_endPos[0];

        navQuery->findNearestPoly(startPos, m_polyPickExt, &m_filter, &m_startRef, 0);
        navQuery->findNearestPoly(endPos, m_polyPickExt, &m_filter, &m_endRef, 0);

        const int maxPolys = query.m_maxPath > 0 ? std::min(query.m_maxPath + 1, MAX_POLYS) : MAX_POLYS;

        navQuery->findPath(m_startRef, m_endRef, startPos, endPos, &m_filter, m_polys, &m_polysCount, maxPolys);
        if (!m_polysCount) return {};

        // TODO KI convert to path
        {
            m_smoothPathCount = 0;
            const int maxSmooth = query.m_maxPath > 0 ? std::min(query.m_maxPath + 1, MAX_SMOOTH) : MAX_SMOOTH;

            // Iterate over the path to find smooth path on the detail mesh surface.
            memcpy(m_polyPath, m_polys, sizeof(dtPolyRef) * m_polysCount);
            dtPolyRef* polys = m_polyPath;
            int polyCount = m_polysCount;

            float iterPos[3], targetPos[3];
            navQuery->closestPointOnPoly(m_startRef, startPos, iterPos, 0);
            navQuery->closestPointOnPoly(polys[polyCount - 1], endPos, targetPos, 0);

            static const float STEP_SIZE = 0.5f;
            static const float SLOP = 0.01f;

            dtVcopy(&m_smoothPath[m_smoothPathCount * 3], iterPos);
            m_smoothPathCount++;

            // Move towards target a small advancement at a time until target reached or
            // when ran out of memory to store the path.
            while (polyCount && m_smoothPathCount < maxSmooth)
            {
                // Find location to steer towards.
                float steerPos[3];
                unsigned char steerPosFlag;
                dtPolyRef steerPosRef;

                if (!getSteerTarget(navQuery, iterPos, targetPos, SLOP,
                    polys, polyCount, steerPos, steerPosFlag, steerPosRef))
                    break;

                bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) ? true : false;
                bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ? true : false;

                // Find movement delta.
                float delta[3], len;
                dtVsub(delta, steerPos, iterPos);
                len = dtMathSqrtf(dtVdot(delta, delta));
                // If the steer target is end of path or off-mesh link, do not move past the location.
                if ((endOfPath || offMeshConnection) && len < STEP_SIZE)
                    len = 1;
                else
                    len = STEP_SIZE / len;
                float moveTgt[3];
                dtVmad(moveTgt, iterPos, delta, len);

                // Move
                float result[3];
                dtPolyRef visited[16];
                int nvisited = 0;
                navQuery->moveAlongSurface(polys[0], iterPos, moveTgt, &m_filter,
                    result, visited, &nvisited, 16);

                polyCount = dtMergeCorridorStartMoved(polys, polyCount, maxPolys, visited, nvisited);
                polyCount = fixupShortcuts(polys, polyCount, navQuery);

                float h = 0;
                navQuery->getPolyHeight(polys[0], result, &h);
                result[1] = h;
                dtVcopy(iterPos, result);

                // Handle end of path and off-mesh links when close enough.
                if (endOfPath && inRange(iterPos, steerPos, SLOP, 1.0f))
                {
                    // Reached end of path.
                    dtVcopy(iterPos, targetPos);
                    if (m_smoothPathCount < maxSmooth)
                    {
                        dtVcopy(&m_smoothPath[m_smoothPathCount * 3], iterPos);
                        m_smoothPathCount++;
                    }
                    break;
                }
                else if (offMeshConnection && inRange(iterPos, steerPos, SLOP, 1.0f))
                {
                    // Reached off-mesh connection.
                    float startPos[3], endPos[3];

                    // Advance the path up to and over the off-mesh connection.
                    dtPolyRef prevRef = 0, polyRef = polys[0];
                    int npos = 0;
                    while (npos < polyCount && polyRef != steerPosRef)
                    {
                        prevRef = polyRef;
                        polyRef = polys[npos];
                        npos++;
                    }
                    for (int i = npos; i < polyCount; ++i)
                        polys[i - npos] = polys[i];
                    polyCount -= npos;

                    // Handle the connection.
                    dtStatus status = navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos, endPos);
                    if (dtStatusSucceed(status))
                    {
                        if (m_smoothPathCount < maxSmooth)
                        {
                            dtVcopy(&m_smoothPath[m_smoothPathCount * 3], startPos);
                            m_smoothPathCount++;
                            // Hack to make the dotted path not visible during off-mesh connection.
                            if (m_smoothPathCount & 1)
                            {
                                dtVcopy(&m_smoothPath[m_smoothPathCount * 3], startPos);
                                m_smoothPathCount++;
                            }
                        }
                        // Move position at the other side of the off-mesh link.
                        dtVcopy(iterPos, endPos);
                        float eh = 0.0f;
                        navQuery->getPolyHeight(polys[0], iterPos, &eh);
                        iterPos[1] = eh;
                    }
                }

                // Store results.
                if (m_smoothPathCount < maxSmooth)
                {
                    dtVcopy(&m_smoothPath[m_smoothPathCount * 3], iterPos);
                    m_smoothPathCount++;
                }
            }
        }

        Path path;
        path.m_startPos = query.m_startPos;
        path.m_endPos = query.m_endPos;
        path.setWaypoints(m_smoothPath, m_smoothPathCount);

        return path;
    }
}

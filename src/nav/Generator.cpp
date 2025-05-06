#include "Generator.h"

#include <string>

#include <glm/ext.hpp>

#include <recastnavigation/Recast.h>
#include <recastnavigation/DetourNavMeshBuilder.h>

#include "RecastContainer.h"
#include "BuildContext.h"
#include "InputGeom.h"

namespace
{
}

namespace nav
{
    Generator::Generator(std::shared_ptr<nav::RecastContainer> container)
        : m_container{ container },
        m_ctx{ std::make_unique<BuildContext>() },
        m_filterLowHangingObstacles(true),
        m_filterLedgeSpans(true),
        m_filterWalkableLowHeightSpans(true),
        m_keepInterResults{true}
    {
        m_settings.reset();
        memset(&m_cfg, 0, sizeof(m_cfg));
    }

    Generator::~Generator()
    {
        cleanup();
    }

    void Generator::cleanup()
    {
        delete[] m_triareas;
        m_triareas = nullptr;

        rcFreeHeightField(m_solid);
        m_solid = nullptr;

        rcFreeCompactHeightfield(m_chf);
        m_chf = nullptr;

        rcFreeContourSet(m_cset);
        m_cset = nullptr;

        rcFreePolyMesh(m_pmesh);
        m_pmesh = nullptr;

        rcFreePolyMeshDetail(m_dmesh);
        m_dmesh = nullptr;

        m_container->cleanup();
    }

    void Generator::addInput(std::unique_ptr<nav::InputGeom> input)
    {
        m_inputCollection.addInput(std::move(input));
    }

    // Build must be done after registering all meshes
    bool Generator::build()
    {
        if (m_inputCollection.empty()) return false;

        auto* ctx = m_ctx.get();

        cleanup();

        //
        // Step 1. Initialize build config.
        //

        // Init build configuration from GUI
        {
            memset(&m_cfg, 0, sizeof(m_cfg));
            m_cfg.cs = m_settings.m_cellSize;
            m_cfg.ch = m_settings.m_cellHeight;
            m_cfg.walkableSlopeAngle = m_settings.m_agentMaxSlope;
            m_cfg.walkableHeight = (int)ceilf(m_settings.m_agentHeight / m_cfg.ch);
            m_cfg.walkableClimb = (int)floorf(m_settings.m_agentMaxClimb / m_cfg.ch);
            m_cfg.walkableRadius = (int)ceilf(m_settings.m_agentRadius / m_cfg.cs);
            m_cfg.maxEdgeLen = (int)(m_settings.m_edgeMaxLen / m_settings.m_cellSize);
            m_cfg.maxSimplificationError = m_settings.m_edgeMaxError;
            m_cfg.minRegionArea = (int)rcSqr(m_settings.m_regionMinSize);		// Note: area = size*size
            m_cfg.mergeRegionArea = (int)rcSqr(m_settings.m_regionMergeSize);	// Note: area = size*size
            m_cfg.maxVertsPerPoly = (int)m_settings.m_vertsPerPoly;
            m_cfg.detailSampleDist = m_settings.m_detailSampleDist < 0.9f ? 0 : m_settings.m_cellSize * m_settings.m_detailSampleDist;
            m_cfg.detailSampleMaxError = m_settings.m_cellHeight * m_settings.m_detailSampleMaxError;

            // Set the area where the navigation will be build.
            // Here the bounds of the input mesh are used, but the
            // area could be specified by an user defined box, etc.
            rcVcopy(m_cfg.bmin, glm::value_ptr(m_inputCollection.getNavMeshBoundsMin()));
            rcVcopy(m_cfg.bmax, glm::value_ptr(m_inputCollection.getNavMeshBoundsMax()));

            rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);
        }

        // Reset build times gathering.
        ctx->resetTimers();

        // Start the build process.
        ctx->startTimer(RC_TIMER_TOTAL);

        ctx->log(RC_LOG_PROGRESS, "Building navigation:");
        ctx->log(RC_LOG_PROGRESS, " - %d x %d cells", m_cfg.width, m_cfg.height);

        //
        // Step 2. Rasterize input polygon soup.
        //

        // Allocate voxel heightfield where we rasterize our input data to.
        m_solid = rcAllocHeightfield();
        if (!m_solid)
        {
            ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
            return false;
        }
        if (!rcCreateHeightfield(ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
        {
            ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
            return false;
        }

        // Allocate array that can hold triangle area types.
        // If you have multiple meshes you need to process, allocate
        // and array which can hold the max number of triangles you need to process.
        {
            int maxTriCount = m_inputCollection.getMaxTriCount();
            delete[] m_triareas;
            m_triareas = new unsigned char[maxTriCount];
            if (!m_triareas)
            {
                ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", maxTriCount);
                return false;
            }
        }

        // Find triangles which are walkable based on their slope and rasterize them.
        // If your input data is multiple meshes, you can transform them here, calculate
        // the are type for each of the meshes and rasterize them.
        for (auto& input : m_inputCollection.getInputs())
        {
            const float* verts = input->getVertices();
            const int nverts = input->getVertexCount();
            const int* tris = input->getTris();
            const int ntris = input->getTriCount();

            memset(m_triareas, 0, ntris * sizeof(unsigned char));

            ctx->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", nverts / 1000.0f, ntris / 1000.0f);

            rcMarkWalkableTriangles(ctx, m_cfg.walkableSlopeAngle, verts, nverts, tris, ntris, m_triareas);
            if (!rcRasterizeTriangles(ctx, verts, nverts, tris, m_triareas, ntris, *m_solid, m_cfg.walkableClimb))
            {
                ctx->log(RC_LOG_ERROR, "buildNavigation: Could not rasterize triangles.");
                return false;
            }
        }

        {
            delete[] m_triareas;
            m_triareas = nullptr;
        }

        //
        // Step 3. Filter walkable surfaces.
        //

        // Once all geometry is rasterized, we do initial pass of filtering to
        // remove unwanted overhangs caused by the conservative rasterization
        // as well as filter spans where the character cannot possibly stand.
        if (m_filterLowHangingObstacles)
            rcFilterLowHangingWalkableObstacles(ctx, m_cfg.walkableClimb, *m_solid);
        if (m_filterLedgeSpans)
            rcFilterLedgeSpans(ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
        if (m_filterWalkableLowHeightSpans)
            rcFilterWalkableLowHeightSpans(ctx, m_cfg.walkableHeight, *m_solid);


        //
        // Step 4. Partition walkable surface to simple regions.
        //

        // Compact the heightfield so that it is faster to handle from now on.
        // This will result more cache coherent data as well as the neighbours
        // between walkable cells will be calculated.
        m_chf = rcAllocCompactHeightfield();
        if (!m_chf)
        {
            ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
            return false;
        }
        if (!rcBuildCompactHeightfield(ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
        {
            ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
            return false;
        }

        if (!m_keepInterResults)
        {
            rcFreeHeightField(m_solid);
            m_solid = 0;
        }

        // Erode the walkable area by agent radius.
        if (!rcErodeWalkableArea(ctx, m_cfg.walkableRadius, *m_chf))
        {
            ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
            return false;
        }

        // (Optional) Mark areas.
        //const ConvexVolume* vols = m_geom->getConvexVolumes();
        //for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
        //    rcMarkConvexPolyArea(ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);

        {
            // Prepare for region partitioning, by calculating distance field along the walkable surface.
            if (!rcBuildDistanceField(ctx, *m_chf))
            {
                ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
                return false;
            }

            // Partition the walkable surface into simple regions without holes.
            if (!rcBuildRegions(ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
            {
                ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
                return false;
            }
        }

        //
        // Step 5. Trace and simplify region contours.
        //

        // Create contours.
        m_cset = rcAllocContourSet();
        if (!m_cset)
        {
            ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
            return false;
        }
        if (!rcBuildContours(ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
        {
            ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
            return false;
        }

        //
        // Step 6. Build polygons mesh from contours.
        //

        // Build polygon navmesh from the contours.
        m_pmesh = rcAllocPolyMesh();
        if (!m_pmesh)
        {
            ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
            return false;
        }
        if (!rcBuildPolyMesh(ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
        {
            ctx->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
            return false;
        }

        //
        // Step 7. Create detail mesh which allows to access approximate height on each polygon.
        //

        m_dmesh = rcAllocPolyMeshDetail();
        if (!m_dmesh)
        {
            ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'.");
            return false;
        }

        if (!rcBuildPolyMeshDetail(ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
        {
            ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
            return false;
        }

        if (!m_keepInterResults)
        {
            rcFreeCompactHeightfield(m_chf);
            m_chf = 0;
            rcFreeContourSet(m_cset);
            m_cset = 0;
        }

        // At this point the navigation mesh data is ready, you can access it from m_pmesh.
        // See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.

        //
        // (Optional) Step 8. Create Detour data from Recast poly mesh.
        //

        // The GUI may allow more max points per polygon than Detour can handle.
        // Only build the detour navmesh if we do not exceed the limit.
        if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
        {
            unsigned char* navData = 0;
            int navDataSize = 0;

            //// Update poly flags from areas.
            //for (int i = 0; i < m_pmesh->npolys; ++i)
            //{
            //    if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
            //        m_pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;

            //    if (m_pmesh->areas[i] == SAMPLE_POLYAREA_GROUND ||
            //        m_pmesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
            //        m_pmesh->areas[i] == SAMPLE_POLYAREA_ROAD)
            //    {
            //        m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
            //    }
            //    else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_WATER)
            //    {
            //        m_pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
            //    }
            //    else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_DOOR)
            //    {
            //        m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
            //    }
            //}

            dtNavMeshCreateParams params;
            memset(&params, 0, sizeof(params));
            params.verts = m_pmesh->verts;
            params.vertCount = m_pmesh->nverts;
            params.polys = m_pmesh->polys;
            params.polyAreas = m_pmesh->areas;
            params.polyFlags = m_pmesh->flags;
            params.polyCount = m_pmesh->npolys;
            params.nvp = m_pmesh->nvp;
            params.detailMeshes = m_dmesh->meshes;
            params.detailVerts = m_dmesh->verts;
            params.detailVertsCount = m_dmesh->nverts;
            params.detailTris = m_dmesh->tris;
            params.detailTriCount = m_dmesh->ntris;
            //params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
            //params.offMeshConRad = m_geom->getOffMeshConnectionRads();
            //params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
            //params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
            //params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
            //params.offMeshConUserID = m_geom->getOffMeshConnectionId();
            //params.offMeshConCount = m_geom->getOffMeshConnectionCount();
            params.walkableHeight = m_settings.m_agentHeight;
            params.walkableRadius = m_settings.m_agentRadius;
            params.walkableClimb = m_settings.m_agentMaxClimb;
            rcVcopy(params.bmin, m_pmesh->bmin);
            rcVcopy(params.bmax, m_pmesh->bmax);
            params.cs = m_cfg.cs;
            params.ch = m_cfg.ch;
            params.buildBvTree = true;

            if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
            {
                ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
                return false;
            }

            m_container->m_navMesh = dtAllocNavMesh();
            if (!m_container->m_navMesh)
            {
                dtFree(navData);
                ctx->log(RC_LOG_ERROR, "Could not create Detour navmesh");
                return false;
            }

            dtStatus status;

            status = m_container->m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
            if (dtStatusFailed(status))
            {
                dtFree(navData);
                ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh");
                return false;
            }

            status = m_container->m_navQuery->init(m_container->m_navMesh, 2048);
            if (dtStatusFailed(status))
            {
                ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh query");
                return false;
            }
        }

        ctx->stopTimer(RC_TIMER_TOTAL);

        // Show performance stats.
        ctx->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", m_pmesh->nverts, m_pmesh->npolys);

        return true;
    }
}

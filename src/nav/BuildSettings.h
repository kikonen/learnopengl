#pragma once

#include "PartitionType.h"

namespace nav
{
    struct BuildSettings
    {
        // Cell size in world units
        float m_cellSize;
        // Cell height in world units
        float m_cellHeight;
        // Agent height in world units
        float m_agentHeight;
        // Agent radius in world units
        float m_agentRadius;
        // Agent max climb in world units
        float m_agentMaxClimb;
        // Agent max slope in degrees
        float m_agentMaxSlope;
        // Region minimum size in voxels.
        // regionMinSize = sqrt(regionMinArea)
        float m_regionMinSize;
        // Region merge size in voxels.
        // regionMergeSize = sqrt(regionMergeArea)
        float m_regionMergeSize;
        // Edge max length in world units
        float m_edgeMaxLen;
        // Edge max error in voxels
        float m_edgeMaxError;
        float m_vertsPerPoly;
        // Detail sample distance in voxels
        float m_detailSampleDist;
        // Detail sample max error in voxel heights.
        float m_detailSampleMaxError;
        // Partition type, see SamplePartitionType
        PartitionType m_partitionType;
        // Bounds of the area to mesh
        float m_navMeshBMin[3];
        float m_navMeshBMax[3];
        // Size of the tiles in voxels
        float m_tileSize;

        void reset()
        {
            m_cellSize = 1.0f;
            m_cellHeight = 1.0f;
            m_agentHeight = 2.0f;
            m_agentRadius = 0.5f;
            m_agentMaxClimb = 1.0f;
            m_agentMaxSlope = 45.0f;
            m_regionMinSize = 8;
            m_regionMergeSize = 20;
            m_edgeMaxLen = 12.0f;
            m_edgeMaxError = 1.3f;
            m_vertsPerPoly = 6.0f;
            m_detailSampleDist = 10.0f;
            m_detailSampleMaxError = 1.0f;
            m_partitionType = PartitionType::SAMPLE_PARTITION_WATERSHED;
        }
    };
}

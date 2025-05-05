#pragma once

namespace mesh
{
    class Mesh;
}

namespace nav
{
    class InputGeom
    {
    public:
        InputGeom(const mesh::Mesh* mesh);
        ~InputGeom();

        /// Method to return static mesh data.
        const mesh::Mesh* getMesh() const { return m_mesh; }
        const float* getMeshBoundsMin() const { return m_meshBMin; }
        const float* getMeshBoundsMax() const { return m_meshBMax; }
        const float* getNavMeshBoundsMin() const { return m_meshBMin; }
        const float* getNavMeshBoundsMax() const { return m_meshBMax; }
        //const rcChunkyTriMesh* getChunkyMesh() const { return m_chunkyMesh; }
        //const BuildSettings* getBuildSettings() const { return m_hasBuildSettings ? &m_buildSettings : 0; }
        bool raycastMesh(float* src, float* dst, float& tmin);

        /// @name Off-Mesh connections.
        ///@{
        int getOffMeshConnectionCount() const { return m_offMeshConCount; }
        const float* getOffMeshConnectionVerts() const { return m_offMeshConVerts; }
        const float* getOffMeshConnectionRads() const { return m_offMeshConRads; }
        const unsigned char* getOffMeshConnectionDirs() const { return m_offMeshConDirs; }
        const unsigned char* getOffMeshConnectionAreas() const { return m_offMeshConAreas; }
        const unsigned short* getOffMeshConnectionFlags() const { return m_offMeshConFlags; }
        const unsigned int* getOffMeshConnectionId() const { return m_offMeshConId; }
        ///@}

    private:
        const mesh::Mesh* m_mesh{ nullptr };

        //rcChunkyTriMesh* m_chunkyMesh;
        //rcMeshLoaderObj* m_mesh;
        float m_meshBMin[3], m_meshBMax[3];
        //BuildSettings m_buildSettings;
        //bool m_hasBuildSettings;

        /// @name Off-Mesh connections.
        ///@{
        static const int MAX_OFFMESH_CONNECTIONS = 256;
        float m_offMeshConVerts[MAX_OFFMESH_CONNECTIONS * 3 * 2];
        float m_offMeshConRads[MAX_OFFMESH_CONNECTIONS];
        unsigned char m_offMeshConDirs[MAX_OFFMESH_CONNECTIONS];
        unsigned char m_offMeshConAreas[MAX_OFFMESH_CONNECTIONS];
        unsigned short m_offMeshConFlags[MAX_OFFMESH_CONNECTIONS];
        unsigned int m_offMeshConId[MAX_OFFMESH_CONNECTIONS];
        int m_offMeshConCount;
        ///@}

        /// @name Convex Volumes.
        ///@{
        //static const int MAX_VOLUMES = 256;
        //ConvexVolume m_volumes[MAX_VOLUMES];
        //int m_volumeCount;
        ///@}
    };
}

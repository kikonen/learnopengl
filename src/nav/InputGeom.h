#pragma once

#include <memory>

#include <glm/glm.hpp>

namespace mesh
{
    class Mesh;
}

namespace nav
{
    class InputGeom
    {
    public:
        InputGeom(
            const glm::mat4& transform,
            const std::shared_ptr<mesh::Mesh> mesh);

        ~InputGeom();

        InputGeom(const InputGeom& other) = delete;
        InputGeom& operator=(InputGeom&& other) = delete;

        void build();

        const float* getVertices() const  { return m_vertices; }
        int getVertexCount() const { return m_vertexCount; }
        const int* getTris() const { return m_tris; }
        int getTriCount() const { return m_triCount; }

        /// Method to return static mesh data.
        //const mesh::Mesh* getMesh() const { return m_mesh; }
        const glm::vec3& getMeshBoundsMin() const { return m_meshBMin; }
        const glm::vec3& getMeshBoundsMax() const { return m_meshBMax; }
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
        const glm::mat4 m_transform;
        const std::shared_ptr<mesh::Mesh> m_mesh{ nullptr };
        bool m_dirty{ true };

        float* m_vertices{ nullptr };
        int m_vertexCount{ 0 };
        int* m_tris{ nullptr };
        int m_triCount{ 0 };

        //rcChunkyTriMesh* m_chunkyMesh;
        //rcMeshLoaderObj* m_mesh;
        glm::vec3 m_meshBMin{ 0.f };
        glm::vec3 m_meshBMax{ 0.f };
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

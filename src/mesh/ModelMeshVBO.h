#pragma once

#include "VertexEntry.h"
#include "IndexEntry.h"

namespace mesh {
    class ModelMesh;

    //
    // https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions#storing-index-and-vertex-data-under-single-buffer
    //
    class ModelMeshVBO {
    public:
        ModelMeshVBO();
        ~ModelMeshVBO();

        void prepare(
            ModelMesh& mesh);

    private:
        void prepareBuffers(
            ModelMesh& mesh);

        void prepareVertex(
            ModelMesh& mesh);

        void prepareIndex(
            ModelMesh& mesh);

    public:
        // NOTE KI absolute offset into vbo
        size_t m_positionOffset = 0;

        // NOTE KI absolute offset into vbo
        size_t m_vertexOffset = 0;

        // NOTE KI absolute offset into vbo
        size_t m_indexOffset = 0;

        std::vector<PositionEntry> m_positionEntries;
        std::vector<VertexEntry> m_vertexEntries;
        std::vector<IndexEntry> m_indexEntries;

    private:
        bool m_prepared = false;
    };
}

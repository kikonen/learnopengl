#pragma once

#include <glm/glm.hpp>

#include "PositionEntry.h"
#include "VertexEntry.h"
#include "IndexEntry.h"

namespace mesh {
    class ModelMesh;
    struct Vertex;

    //
    // https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions#storing-index-and-vertex-data-under-single-buffer
    //
    class ModelVBO {
    public:
        ModelVBO();
        ~ModelVBO();

        void prepare(
            ModelMesh& mesh);

    private:
        void preparePosition(
            const std::vector<Vertex>& positions);

        void prepareVertex(
            const std::vector<Vertex>& vertices);

        void prepareIndex(
            std::vector<glm::uvec3> indeces);

    public:
        // NOTE KI absolute offset into vbo
        size_t m_positionOffset{ 0 };

        // NOTE KI absolute offset into vbo
        size_t m_vertexOffset{ 0 };

        // NOTE KI absolute offset into vbo
        size_t m_indexOffset{ 0 };

        std::vector<PositionEntry> m_positionEntries;
        std::vector<VertexEntry> m_vertexEntries;
        std::vector<IndexEntry> m_indexEntries;

    private:
        bool m_prepared{ false };
    };
}

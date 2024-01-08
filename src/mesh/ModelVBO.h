#pragma once

#include <glm/glm.hpp>

#include "mesh/PositionEntry.h"
#include "mesh/NormalEntry.h"
#include "mesh/TextureEntry.h"
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

        void clear();

        GLsizei getIndexCount() const noexcept {
            return static_cast<GLsizei>(m_indexEntries.size() * 3);
        }

    private:
        void preparePosition(
            const std::vector<Vertex>& vertices);

        void prepareNormal(
            const std::vector<Vertex>& vertices);

        void prepareTexture(
            const std::vector<Vertex>& vertices);

        void prepareIndex(
            std::vector<glm::uvec3> indeces);

    public:
        // NOTE KI absolute offset into VBO
        size_t m_vertexOffset{ 0 };

        // NOTE KI absolute offset into EBO
        size_t m_indexOffset{ 0 };

        std::vector<PositionEntry> m_positionEntries;
        std::vector<NormalEntry> m_normalEntries;
        std::vector<TextureEntry> m_textureEntries;
        std::vector<IndexEntry> m_indexEntries;

    private:
        bool m_prepared{ false };
    };
}

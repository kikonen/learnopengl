#pragma once

#include <string>

#include <glm/glm.hpp>

#include "mesh/PositionEntry.h"

#include "kigl/GLBuffer.h"

namespace kigl {
    struct GLVertexArray;
}

namespace mesh {
    struct Vertex;

    //
    // https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions#storing-index-and-vertex-data-under-single-buffer
    //
    class PositionVBO {
    public:
        PositionVBO(std::string_view name);
        ~PositionVBO();

        // @return base *offset* into buffer
        size_t addPositions(
            const std::vector<Vertex>& positions);

        // @return base *offset* into buffer
        size_t addEntry(const PositionEntry& entry);

        void reserveSize(size_t count);

        void prepareVAO(kigl::GLVertexArray& vao);
        void updateVAO(kigl::GLVertexArray& vao);

        void clear();

        // @return base *offset* into buffer (for next new entry)
        size_t getBaseOffset() const noexcept {
            return m_entries.size();
        }

    private:
        bool m_prepared{ false };

        kigl::GLBuffer m_vbo;
        size_t m_lastBufferSize = 0;

        std::vector<PositionEntry> m_entries;
    };
}

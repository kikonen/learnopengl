#pragma once

#include <string_view>
#include <span>

#include "kigl/GLBuffer.h"

#include "mesh/Index.h"

#include "IndexEntry.h"

namespace kigl {
    struct GLVertexArray;
}

namespace mesh
{
    class VertexIndexEBO {
    public:
        VertexIndexEBO(
            std::string_view name);

        // @return baseIndex
        uint32_t reserveIndeces(size_t count);

        void updateIndeces(
            uint32_t baseIndex,
            std::span<mesh::Index> indeces) noexcept;

        void clear();

        void prepareVAO(kigl::GLVertexArray& vao);
        void updateVAO(kigl::GLVertexArray& vao);

    private:
        kigl::GLBuffer m_ebo;

        std::vector<IndexEntry> m_entries;
        size_t m_lastSize = 0;
    };
}

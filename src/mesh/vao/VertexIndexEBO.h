#pragma once

#include <string_view>
#include <vector>

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

        size_t addIndeces(std::vector<mesh::Index> indeces);
        void clear();

        void prepareVAO(kigl::GLVertexArray& vao);
        void updateVAO(kigl::GLVertexArray& vao);

    private:
        kigl::GLBuffer m_ebo;

        std::vector<IndexEntry> m_entries;
        size_t m_lastSize = 0;
    };
}

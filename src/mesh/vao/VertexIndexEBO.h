#pragma once

#include <string_view>
#include <span>
#include <vector>
#include <tuple>

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
            std::span<mesh::Index32> indeces) noexcept;

        void clear();

        void prepareVAO(kigl::GLVertexArray& vao);
        void updateVAO(kigl::GLVertexArray& vao);

    protected:
        bool updateSpan(
            kigl::GLVertexArray& vao,
            size_t updateIndex,
            size_t updateCount);

    private:
        kigl::GLBuffer m_ebo;

        std::vector < std::pair<uint32_t, size_t>> m_dirty;
        std::vector<IndexEntry32> m_entries;
    };
}

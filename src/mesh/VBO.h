#pragma once

#include <string>
#include <string_view>

#include <glm/glm.hpp>

#include "kigl/GLBuffer.h"

namespace kigl {
    struct GLVertexArray;
}

namespace mesh {
    //
    // https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions#storing-index-and-vertex-data-under-single-buffer
    //
    template<typename T_Vertex, typename T_Entry>
    class VBO {
    public:
        VBO(
            int attr,
            int binding,
            std::string_view name);

        virtual ~VBO();

        // @return base *offset* into buffer
        size_t addVertices(
            const std::vector<T_Entry>& vertices);

        size_t addVertex(
            const T_Entry& vertex);

        // @return base *offset* into buffer
        size_t addEntry(const T_Entry& entry);

        void reserveSize(size_t count);

        virtual void prepareVAO(kigl::GLVertexArray& vao) = 0;

        void updateVAO(kigl::GLVertexArray& vao);

        void clear();

        // @return base *offset* into buffer (for next new entry)
        size_t getBaseOffset() const noexcept {
            return m_entries.size();
        }

    protected:
        const int m_binding;
        const int m_attr;

        bool m_prepared{ false };

        kigl::GLBuffer m_vbo;
        size_t m_lastBufferSize = 0;

        std::vector<T_Entry> m_entries;
    };
}

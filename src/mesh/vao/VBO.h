#pragma once

#include <string>
#include <string_view>
#include <span>

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
            std::string_view name,
            int attr,
            int binding);

        virtual ~VBO();

        // @return base *index* into entries
        uint32_t reserveVertices(size_t count);

        void updateVertices(
            uint32_t baseIndex,
            const std::span<T_Vertex>& vertices);

        virtual T_Entry convertVertex(
            const T_Vertex& vertex) = 0;

        void reserveSize(size_t count);

        virtual void prepareVAO(kigl::GLVertexArray& vao) = 0;

        void updateVAO(kigl::GLVertexArray& vao);

        void clear();

        // @return base *index* into buffer (for next new entry)
        uint32_t getBaseIndex() const noexcept {
            return static_cast<uint32_t>(m_entries.size());
        }

    protected:
        const int m_binding;
        const int m_attr;

        bool m_prepared{ false };

        kigl::GLBuffer m_vbo;
        size_t m_lastBufferSize = 0;

        kigl::GLVertexArray* m_vao{ nullptr };

        std::vector<T_Entry> m_entries;
    };
}

#include "VertexNormalVBO.h"

#include "VBO_impl.h"

namespace {
    constexpr size_t VERTEX_BLOCK_SIZE = 1000;
    constexpr size_t VERTEX_BLOCK_COUNT = 10000;

    constexpr size_t MAX_VERTEX_COUNT = VERTEX_BLOCK_SIZE * VERTEX_BLOCK_COUNT;
}

namespace mesh {
    VertexNormalVBO::VertexNormalVBO(
        std::string_view name,
        int attr,
        int tangentAttr,
        int binding)
        : VBO{ name, attr, binding },
        m_tangentAttr{ tangentAttr }
    {}

    NormalEntry VertexNormalVBO::convertVertex(
        const Vertex& vertex)
    {
        return { vertex.normal, vertex.tangent };
    }

    void VertexNormalVBO::prepareVAO(kigl::GLVertexArray& vao)
    {
        constexpr size_t sz = sizeof(NormalEntry);
        {
            m_entries.reserve(VERTEX_BLOCK_SIZE);
            m_vbo.createEmpty(VERTEX_BLOCK_SIZE * sz, GL_DYNAMIC_STORAGE_BIT);
        }

        {
            // "Tile" based GPU can benefit from having separate position stream VBO for improved caching
            // https://solidpixel.github.io/2022/07/21/vertexpacking.html
            // https://www.intel.com/content/www/us/en/developer/articles/guide/developer-and-optimization-guide-for-intel-processor-graphics-gen11-api.html

            glVertexArrayVertexBuffer(vao, m_binding, m_vbo, 0, sizeof(NormalEntry));
            {
                glEnableVertexArrayAttrib(vao, m_attr);
                glEnableVertexArrayAttrib(vao, m_tangentAttr);

                // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                //

                // normal attr
                glVertexArrayAttribFormat(vao, m_attr, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(NormalEntry, normal));

                // tangent attr
                glVertexArrayAttribFormat(vao, m_tangentAttr, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(NormalEntry, tangent));

                glVertexArrayAttribBinding(vao, m_attr, m_binding);
                glVertexArrayAttribBinding(vao, m_tangentAttr, m_binding);

                // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                glVertexArrayBindingDivisor(vao, m_binding, 0);
            }
        }
    }
}

#include "VertexTextureVBO.h"

#include "VBO_impl.h"

namespace mesh {
    VertexTextureVBO::VertexTextureVBO(
        std::string_view name,
        int attr,
        int binding)
        : VBO(name, attr, binding)
    {}

    TextureEntry VertexTextureVBO::convertVertex(
        const Vertex& vertex)
    {
        return { vertex.texCoord };
    }

    void VertexTextureVBO::prepareVAO(kigl::GLVertexArray& vao)
    {
        constexpr size_t sz = sizeof(TextureEntry);
        {
            m_entries.reserve(VERTEX_BLOCK_SIZE);
            m_vbo.createEmpty(VERTEX_BLOCK_SIZE * sz, GL_DYNAMIC_STORAGE_BIT);
        }

        {
            // "Tile" based GPU can benefit from having separate position stream VBO for improved caching
            // https://solidpixel.github.io/2022/07/21/vertexpacking.html
            // https://www.intel.com/content/www/us/en/developer/articles/guide/developer-and-optimization-guide-for-intel-processor-graphics-gen11-api.html

            glVertexArrayVertexBuffer(vao, m_binding, m_vbo, 0, sz);
            {
                glEnableVertexArrayAttrib(vao, m_attr);

                // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                //

                // texture attr
                // NOTE KI tex coords are in [0.0, 1.0] range, thus normalized 16bit value should be enough for them
                //glVertexArrayAttribFormat(vao, m_attr, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(TextureEntry, u_texCoord));
                glVertexArrayAttribFormat(vao, m_attr, 2, GL_FLOAT, GL_FALSE, offsetof(TextureEntry, u_texCoord));

                glVertexArrayAttribBinding(vao, m_attr, m_binding);

                // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                glVertexArrayBindingDivisor(vao, m_binding, 0);
            }
        }
    }
}

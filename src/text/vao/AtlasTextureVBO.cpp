#include "AtlasTextureVBO.h"

#include "mesh/vao/VBO_impl.h"
#include "mesh/vao/TextureEntry.h"

namespace text {
    AtlasTextureVBO::AtlasTextureVBO(
        std::string_view name,
        int attr,
        int binding)
        : VBO(name, attr, binding)
    {}

    mesh::TextureEntry AtlasTextureVBO::convertVertex(
        const glm::vec2& vertex)
    {
        return { vertex };
    }

    void AtlasTextureVBO::prepareVAO(kigl::GLVertexArray& vao)
    {
        constexpr size_t sz = sizeof(mesh::TextureEntry);
        {
            m_entries.reserve(mesh::VERTEX_BLOCK_SIZE);
            m_vbo.createEmpty(mesh::VERTEX_BLOCK_SIZE * sz, GL_DYNAMIC_STORAGE_BIT);
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
                // vertex attr
                glVertexArrayAttribFormat(vao, m_attr, 2, GL_FLOAT, GL_FALSE, offsetof(mesh::TextureEntry, u_texCoord));

                glVertexArrayAttribBinding(vao, m_attr, m_binding);

                // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                glVertexArrayBindingDivisor(vao, m_binding, 0);
            }
        }
    }
}

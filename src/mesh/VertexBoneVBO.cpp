#include "VertexBoneVBO.h"

#include "VBO_impl.h"

namespace {
    constexpr size_t VERTEX_BLOCK_SIZE = 1000;
    constexpr size_t VERTEX_BLOCK_COUNT = 10000;

    constexpr size_t MAX_VERTEX_COUNT = VERTEX_BLOCK_SIZE * VERTEX_BLOCK_COUNT;
}

namespace mesh {
    VertexBoneVBO::VertexBoneVBO(
        std::string_view name,
        int boneIdAttr,
        int weightAttr,
        int binding)
        : VBO(name, boneIdAttr, binding),
        m_weightAttr{weightAttr}
    {}

    BoneEntry VertexBoneVBO::convertVertex(
        const animation::VertexBone& bone)
    {
        return { bone };
    }

    void VertexBoneVBO::prepareVAO(kigl::GLVertexArray& vao)
    {
        constexpr size_t sz = sizeof(BoneEntry);
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

                // boneId attr
                glVertexArrayAttribFormat(vao, m_attr, 4, GL_UNSIGNED_INT, GL_FALSE, offsetof(BoneEntry, m_boneIds));

                // weight attr
                glVertexArrayAttribFormat(vao, m_weightAttr, 4, GL_FLOAT, GL_FALSE, offsetof(BoneEntry, m_weights));

                glVertexArrayAttribBinding(vao, m_attr, m_binding);

                // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                glVertexArrayBindingDivisor(vao, m_binding, 0);
            }
        }
    }
}

#include "VertexJointVBO.h"

#include "VBO_impl.h"

namespace mesh {
    VertexJointVBO::VertexJointVBO(
        std::string_view name,
        int jointIdAttr,
        int weightAttr,
        int binding)
        : VBO(name, jointIdAttr, binding),
        m_weightAttr{weightAttr}
    {}

    JointEntry VertexJointVBO::convertVertex(
        const animation::VertexJoint& joint)
    {
        return { joint };
    }

    void VertexJointVBO::prepareVAO(kigl::GLVertexArray& vao)
    {
        constexpr size_t sz = sizeof(JointEntry);
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
                glEnableVertexArrayAttrib(vao, m_weightAttr);

                // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                //

                // jointId attr
                glVertexArrayAttribIFormat(vao, m_attr, 4, GL_UNSIGNED_INT, offsetof(JointEntry, m_jointIds));

                // weight attr
                glVertexArrayAttribFormat(vao, m_weightAttr, 4, GL_FLOAT, GL_FALSE, offsetof(JointEntry, m_weights));

                glVertexArrayAttribBinding(vao, m_attr, m_binding);
                glVertexArrayAttribBinding(vao, m_weightAttr, m_binding);

                // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                glVertexArrayBindingDivisor(vao, m_binding, 0);
            }
        }
    }
}

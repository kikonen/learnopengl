#include "VertexVBO.h"

#include "glm/glm.hpp"

#include "shader/Shader.h"

#include "kigl/GLVertexArray.h"

#include "mesh/Vertex.h"

#include "VBO_impl.h"


namespace mesh {
    VertexVBO::VertexVBO(
        std::string_view name,
        int binding)
        : VBO(name, -1, binding)
    {}

    VertexEntry VertexVBO::convertVertex(
        const Vertex& vertex)
    {
        return { vertex };
    }

    void VertexVBO::prepareVAO(kigl::GLVertexArray& vao)
    {
        {
            m_entries.reserve(VERTEX_BLOCK_SIZE);
            m_vbo.createEmpty(VERTEX_BLOCK_SIZE * sizeof(VertexEntry), GL_DYNAMIC_STORAGE_BIT);
        }

        // "Tile" based GPU can benefit from having separate position stream VBO for improved caching
        // https://solidpixel.github.io/2022/07/21/vertexpacking.html
        // https://www.intel.com/content/www/us/en/developer/articles/guide/developer-and-optimization-guide-for-intel-processor-graphics-gen11-api.html
        //
        // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        {

            glVertexArrayVertexBuffer(vao, m_binding, m_vbo, 0, sizeof(VertexEntry));
            {
                {
                    glEnableVertexArrayAttrib(vao, ATTR_POS);

                    glVertexArrayAttribFormat(vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, offsetof(VertexEntry, u_x));
                    glVertexArrayAttribBinding(vao, ATTR_POS, m_binding);
                }
                {
                    glEnableVertexArrayAttrib(vao, ATTR_TEX);

                    // NOTE KI tex coords are in [0.0, 1.0] range, thus normalized 16bit value should be enough for them
                    //glVertexArrayAttribFormat(vao, ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(VertexEntry, u_texCoord));
                    glVertexArrayAttribFormat(vao, ATTR_TEX, 2, GL_FLOAT, GL_FALSE, offsetof(VertexEntry, u_texCoord));

                    glVertexArrayAttribBinding(vao, ATTR_TEX, m_binding);
                }
                {
                    glEnableVertexArrayAttrib(vao, ATTR_NORMAL);

                    // https://gamedev.stackexchange.com/questions/46304/how-does-gl-int-2-10-10-10-rev-work-for-color-data
                    //glVertexArrayAttribFormat(vao, ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, offsetof(VertexEntry, u_normal));
                    //glVertexArrayAttribFormat(vao, ATTR_NORMAL, 3, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(VertexEntry, u_normal));
                    glVertexArrayAttribFormat(vao, ATTR_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_FALSE, offsetof(VertexEntry, u_normal));
                    glVertexArrayAttribBinding(vao, ATTR_NORMAL, m_binding);
                }
                {
                    glEnableVertexArrayAttrib(vao, ATTR_TANGENT);

                    //glVertexArrayAttribFormat(vao, ATTR_TANGENT, 3, GL_FLOAT, GL_FALSE, offsetof(VertexEntry, u_tangent));
                    //glVertexArrayAttribFormat(vao, ATTR_TANGENT, 3, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(VertexEntry, u_tangent));
                    glVertexArrayAttribFormat(vao, ATTR_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_FALSE, offsetof(VertexEntry, u_tangent));
                    glVertexArrayAttribBinding(vao, ATTR_TANGENT, m_binding);
                }
            }

            // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
            // https://www.khronos.org/opengl/wiki/Vertex_Specification
            glVertexArrayBindingDivisor(vao, m_binding, 0);
        }
    }

    AABB VertexVBO::calculateAABB() const noexcept
    {
        AABB aabb{ true };

        for (auto&& entry : m_entries)
        {
            aabb.minmax({ entry.u_x, entry.u_y, entry.u_z });
        }

        aabb.updateVolume();

        return aabb;
    }
}

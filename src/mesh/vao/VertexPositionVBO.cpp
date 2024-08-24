#include "VertexPositionVBO.h"

#include "glm/glm.hpp"

#include "shader/Shader.h"

#include "kigl/GLVertexArray.h"

#include "mesh/Vertex.h"

#include "VBO_impl.h"


namespace mesh {
    VertexPositionVBO::VertexPositionVBO(
        std::string_view name,
        int attr,
        int binding)
        : VBO(name, attr, binding)
    {}

    PositionEntry VertexPositionVBO::convertVertex(
        const Vertex& vertex)
    {
        return { vertex.pos };
    }

    void VertexPositionVBO::prepareVAO(kigl::GLVertexArray& vao)
    {
        {
            m_entries.reserve(VERTEX_BLOCK_SIZE);
            m_vbo.createEmpty(VERTEX_BLOCK_SIZE * sizeof(PositionEntry), GL_DYNAMIC_STORAGE_BIT);
        }

        {
            // "Tile" based GPU can benefit from having separate position stream VBO for improved caching
            // https://solidpixel.github.io/2022/07/21/vertexpacking.html
            // https://www.intel.com/content/www/us/en/developer/articles/guide/developer-and-optimization-guide-for-intel-processor-graphics-gen11-api.html

            glVertexArrayVertexBuffer(vao, m_binding, m_vbo, 0, sizeof(PositionEntry));
            {
                glEnableVertexArrayAttrib(vao, m_attr);

                // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                //
                // vertex attr
                glVertexArrayAttribFormat(vao, m_attr, 3, GL_FLOAT, GL_FALSE, offsetof(PositionEntry, x));

                glVertexArrayAttribBinding(vao, m_attr, m_binding);

                // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                glVertexArrayBindingDivisor(vao, m_binding, 0);
            }
        }
    }

    AABB VertexPositionVBO::calculateAABB() const noexcept
    {
        AABB aabb{ true };

        for (auto&& vertex : m_entries)
        {
            aabb.minmax({ vertex.x, vertex.y, vertex.z });
        }

        aabb.updateVolume();

        return aabb;
    }
}

#include "VertexPositionVBO.h"

#include "glm/glm.hpp"

#include "asset/Shader.h"

#include "kigl/GLVertexArray.h"

#include "mesh/Vertex.h"

namespace {
    constexpr size_t VERTEX_BLOCK_SIZE = 1000;
    constexpr size_t VERTEX_BLOCK_COUNT = 10000;

    constexpr size_t MAX_VERTEX_COUNT = VERTEX_BLOCK_SIZE * VERTEX_BLOCK_COUNT;
}

namespace mesh {
    VertexPositionVBO::VertexPositionVBO(std::string_view name)
        : m_vbo{ name }
    {}

    VertexPositionVBO::~VertexPositionVBO()
    {
    }

    size_t VertexPositionVBO::addPositions(
        const glm::vec3& posOffset,
        const std::vector<Vertex>& positions)
    {
        const size_t baseIndex = m_entries.size();
        const size_t baseOffset = baseIndex * sizeof(PositionEntry);

        reserveSize(positions.size());
        for (const auto& vertex : positions) {
            addEntry(posOffset, { vertex.pos });
        }

        return baseOffset;
    }

    size_t VertexPositionVBO::addEntry(
        const glm::vec3& posOffset,
        const PositionEntry& entry)
    {
        if (m_entries.size() >= MAX_VERTEX_COUNT) {
            throw std::runtime_error{ fmt::format("MAX_VERTEX_COUNT: {}", MAX_VERTEX_COUNT) };
        }

        const size_t baseIndex = m_entries.size();
        const size_t baseOffset = baseIndex * sizeof(PositionEntry);

        reserveSize(1);
        auto& stored = m_entries.emplace_back(entry);
        //stored += posOffset;

        return baseOffset;
    }

    // https://paroj.github.io/gltut/Basic%20Optimization.html
    void VertexPositionVBO::reserveSize(size_t count)
    {
        size_t size = m_entries.size() + std::max(VERTEX_BLOCK_SIZE, count) + VERTEX_BLOCK_SIZE;
        size += VERTEX_BLOCK_SIZE - size % VERTEX_BLOCK_SIZE;
        size = std::min(size, MAX_VERTEX_COUNT);
        m_entries.reserve(size);
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

            glVertexArrayVertexBuffer(vao, VBO_POSITION_BINDING, m_vbo, 0, sizeof(PositionEntry));
            {
                glEnableVertexArrayAttrib(vao, ATTR_POS);

                // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                //
                // vertex attr
                glVertexArrayAttribFormat(vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, offsetof(PositionEntry, x));

                glVertexArrayAttribBinding(vao, ATTR_POS, VBO_POSITION_BINDING);

                // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                glVertexArrayBindingDivisor(vao, VBO_POSITION_BINDING, 0);
            }
        }
    }

    void VertexPositionVBO::updateVAO(kigl::GLVertexArray& vao)
    {
        const size_t index = m_lastBufferSize;
        const size_t totalCount = m_entries.size();

        if (index == totalCount) return;
        if (totalCount == 0) return;

        {
            constexpr size_t sz = sizeof(PositionEntry);
            size_t updateIndex = index;

            // NOTE KI *reallocate* SSBO if needed
            if (m_vbo.m_size < totalCount * sz) {
                m_vbo.resizeBuffer(m_entries.capacity() * sz);
                glVertexArrayVertexBuffer(vao, VBO_POSITION_BINDING, m_vbo, 0, sizeof(PositionEntry));
                updateIndex = 0;
            }

            const size_t updateCount = totalCount - updateIndex;

            m_vbo.update(
                updateIndex * sz,
                updateCount * sz,
                &m_entries[updateIndex]);
        }

        m_lastBufferSize = totalCount;
    }

    void VertexPositionVBO::clear()
    {
        m_entries.clear();
        m_lastBufferSize = 0;
    }

    AABB VertexPositionVBO::calculateAABB() const noexcept
    {
        glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

        for (auto&& vertex : m_entries)
        {
            minAABB.x = std::min(minAABB.x, vertex.x);
            minAABB.y = std::min(minAABB.y, vertex.y);
            minAABB.z = std::min(minAABB.z, vertex.z);

            maxAABB.x = std::max(maxAABB.x, vertex.x);
            maxAABB.y = std::max(maxAABB.y, vertex.y);
            maxAABB.z = std::max(maxAABB.z, vertex.z);
        }

        return { minAABB, maxAABB, false };
    }
}

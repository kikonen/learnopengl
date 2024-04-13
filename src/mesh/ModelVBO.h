#pragma once

#include <glm/glm.hpp>

#include "asset/AABB.h"

#include "mesh/PositionEntry.h"

namespace mesh {
    class ModelMesh;
    struct Vertex;

    //
    // https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions#storing-index-and-vertex-data-under-single-buffer
    //
    class ModelVBO {
    public:
        ModelVBO();
        ~ModelVBO();

        void prepare(
            ModelMesh& mesh);

        AABB calculateAABB() const noexcept;

        void clear();

        inline uint32_t getBaseVertex() const noexcept {
            return static_cast<uint32_t>(m_positionVboOffset / sizeof(mesh::PositionEntry));
        }

        inline uint32_t getBaseIndex() const noexcept {
            return static_cast<uint32_t>(m_indexEboOffset / sizeof(GLuint));
        }

        inline uint32_t getIndexCount() const noexcept {
            return m_indexCount;
        }

    public:
        // NOTE KI absolute offset into position VBO
        size_t m_positionVboOffset{ 0 };

        // NOTE KI absolute offset into EBO
        size_t m_indexEboOffset{ 0 };

        uint32_t m_indexCount{ 0 };

        glm::vec3 m_meshPositionOffset{ 0.f };

        ModelMesh* m_mesh{ nullptr };

    private:
        bool m_prepared{ false };
    };
}

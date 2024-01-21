#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"


class Program;


namespace render {
    struct BatchCommand {
        const Program* m_program{ nullptr };

        const kigl::GLVertexArray* m_vao{ nullptr };
        backend::DrawOptions m_drawOptions;

        int m_index{ 0 };
        int m_drawCount{ 0 };
        int m_instancedCount{ 1 };

        std::vector<int> m_entityIndeces;

        void reserve(size_t size) {
            m_entityIndeces.reserve(size);
        }

        inline bool empty() const noexcept {
            return m_drawCount == 0;
        }

        inline void clear() noexcept {
            m_drawCount = 0;
            m_index = 0;
            m_instancedCount = 1;
            m_entityIndeces.clear();
        }

        void addIndex(int index) noexcept {
            m_entityIndeces.emplace_back(index);
        }
    };
}

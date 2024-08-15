#pragma once

#include <vector>
#include <mutex>
#include <atomic>

#include <glm/glm.hpp>

#include "kigl/GLBuffer.h"

struct UpdateContext;
class RenderContext;

struct TransformSSBO;

#include <glm/glm.hpp>

namespace mesh {
    class TransformRegistry {
    public:
        static TransformRegistry& get() noexcept;

        TransformRegistry();
        TransformRegistry& operator=(const TransformRegistry&) = delete;

        ~TransformRegistry();

        void prepare();

        void updateRT(const UpdateContext& ctx);

    private:
        uint32_t registerTransform(const glm::mat4& transform);
        size_t getBaseIndex() { return m_transforms.size(); }

        void updateBuffer();

    private:
        std::atomic<bool> m_dirtyFlag;
        std::mutex m_lock{};

        std::vector<glm::mat4> m_transforms;

        std::vector<glm::mat4> m_transformEntries;

        size_t m_lastSize{ 0 };

        kigl::GLBuffer m_ssbo{ "transforms_ssbo" };
    };
}

#include "TransformRegistry.h"

#include <fmt/format.h>

#include "util/Thread.h"

#include "asset/SSBO.h"

#include "mesh/TransformSSBO.h"

namespace {
    constexpr size_t BLOCK_SIZE = 10;
    constexpr size_t BLOCK_COUNT = 1000;

    constexpr size_t MAX_COUNT = BLOCK_SIZE * BLOCK_COUNT;

    static mesh::TransformRegistry s_registry;
}

namespace mesh {
    TransformRegistry& TransformRegistry::get() noexcept
    {
        return s_registry;
    }

    TransformRegistry::TransformRegistry()
    {
        m_transforms.reserve(BLOCK_SIZE);
        m_transformEntries.reserve(BLOCK_SIZE);

        // NOTE KI *reserve* index 0 for identity matrix
        registerTransform(glm::mat4{ 1.f });
    }

    TransformRegistry::~TransformRegistry() = default;

    uint32_t TransformRegistry::registerTransform(const glm::mat4& transform)
    {
        std::lock_guard lock(m_lock);

        const uint32_t index = static_cast<uint32_t>(m_transforms.size());
        m_transforms.emplace_back(transform);

        return index;
    }

    void TransformRegistry::updateRT(const UpdateContext& ctx)
    {
        std::lock_guard lock(m_lock);

        updateBuffer();
    }

    void TransformRegistry::prepare()
    {
        m_ssbo.createEmpty(BLOCK_SIZE * sizeof(TransformSSBO), GL_DYNAMIC_STORAGE_BIT);
    }

    void TransformRegistry::bind(
        const RenderContext& ctx)
    {
        m_ssbo.bindSSBO(SSBO_MESH_TRANSFORMS);
    }

    void TransformRegistry::updateBuffer()
    {
        const size_t index = m_lastSize;
        const size_t totalCount = m_transforms.size();

        if (index == totalCount) return;
        if (totalCount == 0) return;

        {
            // NOTE KI update m_materialsSSBO from *index*, not *updateIndex* point
            // - otherwise entries are multiplied, and indexed incorrectly
            for (size_t i = index; i < totalCount; i++) {
                auto& transform = m_transforms[i];
                m_transformEntries.emplace_back(transform);
            }
        }

        {
            constexpr size_t sz = sizeof(TransformSSBO);

            size_t updateIndex = index;

            // NOTE KI *reallocate* SSBO if needed
            if (m_ssbo.m_size < totalCount * sz) {
                m_ssbo.resizeBuffer(m_transformEntries.capacity() * sz);
                m_ssbo.bindSSBO(SSBO_MESH_TRANSFORMS);
                updateIndex = 0;
            }

            const size_t updateCount = totalCount - updateIndex;

            m_ssbo.update(
                updateIndex * sz,
                updateCount * sz,
                &m_transformEntries[updateIndex]);
        }

        m_lastSize = totalCount;
    }
}

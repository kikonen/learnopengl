#pragma once

#include <vector>
#include <unordered_map>
#include <span>

#include "kigl/GLBuffer.h"
#include "kigl/GLFence.h"

#include "util/BufferReference.h"

#include "backend/DrawOptions.h"

#include "DrawableInfo.h"

namespace render
{
    struct InstanceSSBO;

    class InstanceRegistry
    {
    public:
        static void init() noexcept;
        static void release() noexcept;
        static InstanceRegistry& get() noexcept;

        InstanceRegistry();
        ~InstanceRegistry();

        void clear();
        void prepare();

        // @return ref to buffer
        util::BufferReference allocate(uint32_t count);
        // @return null ref
        util::BufferReference release(util::BufferReference ref);

        //// Called at load time when meshes are registered
        //// @return persistent instance index
        //uint32_t registerDrawable(const DrawableInfo& info);

        std::span<const render::DrawableInfo> getRange(
            const util::BufferReference ref) const noexcept;

        std::span<render::DrawableInfo> modifyRange(
            util::BufferReference ref) noexcept;

        void markDirtyAll() noexcept;
        void markDirty(util::BufferReference ref) noexcept;

        void updateInstances();

        // Upload to GPU (call once per frame after updateTransforms)
        void upload();

        // Bind for rendering
        void bind();

        size_t getDrawableCount() const { return m_drawables.size(); }

    private:
        kigl::GLBuffer m_ssbo{ "instance_ssbo" };
        kigl::GLFence m_fence{ "fence_entity" };
        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useFenceDebug{ false };

        std::vector<DrawableInfo> m_drawables;

        // { size: [index, ...] }
        std::unordered_map<size_t, std::vector<uint32_t>> m_freeSlots;
        std::vector<util::BufferReference> m_dirtySlots;

        std::vector<render::InstanceSSBO> m_instances;
        std::vector<util::BufferReference> m_dirtyInstances;

        //bool m_dirty{ true };
        size_t m_uploadedCount{ 0 };
    };
}

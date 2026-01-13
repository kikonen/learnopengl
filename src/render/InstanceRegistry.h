#pragma once

#include <vector>
#include <span>

#include "kigl/GLBuffer.h"
#include "kigl/GLFence.h"

#include "util/BufferReference.h"
#include "util/DirtySet.h"
#include "util/SlotAllocator.h"

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

        void beginFrame();
        void endFrame();

        // @return ref to buffer
        util::BufferReference allocate(size_t count);
        // @return null ref
        util::BufferReference release(util::BufferReference ref);

        std::span<const render::DrawableInfo> getRange(
            const util::BufferReference ref) const noexcept;

        std::span<render::DrawableInfo> modifyRange(
            util::BufferReference ref) noexcept;

        void markDirtyAll() noexcept;
        void markDirty(util::BufferReference ref) noexcept;

        void prepareInstances(util::BufferReference ref) noexcept;
        void updateInstances(util::BufferReference ref) noexcept;

        //void updateInstances();

        // Upload to GPU (call once per frame after updateTransforms)
        void upload();

        void upload(util::BufferReference ref);

        size_t getDrawableCount() const { return m_drawables.size(); }

    private:
        void resizeBuffer(size_t totalCount);

    private:
        bool m_debug{ false };

        std::vector<DrawableInfo> m_drawables;

        util::SlotAllocator m_slotAllocator;
        util::DirtySet<util::BufferReference> m_dirtySlots;

        std::vector<render::InstanceSSBO> m_instances;

        kigl::GLBuffer m_ssbo{ "instances" };
        kigl::GLFence m_fence{ "instances_fence" };

        bool m_needUpload{ false };
        size_t m_uploadedCount{ 0 };
    };
}

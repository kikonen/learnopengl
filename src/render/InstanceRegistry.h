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

        void clear();
        void prepare();

        // Called at load time when meshes are registered
        // @return persistent instance index
        uint32_t registerDrawable(const DrawableInfo& info);

        // Called when entity is destroyed
        void unregisterDrawable(uint32_t instanceIndex);

        std::span<const render::DrawableInfo> getRange(
            const util::BufferReference ref) const noexcept;

        std::span<render::DrawableInfo> modifyRange(
            util::BufferReference ref) noexcept;

        void updateInstances();

        // Upload to GPU (call once per frame after updateTransforms)
        void upload();

        // Bind for rendering
        void bind();

        // Accessors for Batch to build draw commands
        const DrawableInfo& getDrawableInfo(uint32_t index) const
        {
            return m_drawables[index];
        }

        size_t getDrawableCount() const { return m_drawables.size(); }

        // Get instance index for a specific entity + mesh combination
        uint32_t getInstanceIndex(uint32_t entityIndex, uint32_t meshId) const;

    private:
        kigl::GLBuffer m_ssbo{ "instance_ssbo" };
        kigl::GLFence m_fence{ "fence_entity" };
        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useFenceDebug{ false };

        std::vector<DrawableInfo> m_drawables;

        // CPU staging
        std::vector<render::InstanceSSBO> m_instances;


        // Lookup: (entityIndex, meshId) → instance index
        std::unordered_map<uint64_t, uint32_t> m_lookupMap;

        bool m_dirty{ true };
        size_t m_uploadedCount{ 0 };
    };
}

#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <span>

#include "kigl/GLBuffer.h"

namespace animation {
    class AnimationSystem;
    class BoneRegistry;

    class BoneBuffer {
        friend AnimationSystem;

    public:
        BoneBuffer(BoneRegistry* boneRegistry);
        BoneBuffer& operator=(const BoneBuffer&) = delete;

        ~BoneBuffer();

        void clear();

        void shutdown();
        void prepare();

    protected:
        void updateRT();

    private:
        void updateBuffer();

        bool updateSpan(
            const std::vector<glm::mat4>& m_snapshot,
            size_t updateIndex,
            size_t updateCount);

    private:
        BoneRegistry* const m_boneRegistry;

        kigl::GLBuffer m_ssbo{ "bone_palette_ssbo" };

        size_t m_frameSkipCount{ 0 };

        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useDebugFence{ false };
    };
}

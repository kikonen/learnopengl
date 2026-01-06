#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <span>
#include <memory>

#include "kigl/GLBuffer.h"
#include "kigl/GLFence.h"

#include "util/BufferReference.h"

namespace animation {
    class AnimationSystem;
    class JointRegistry;

    struct JointTransformSSBO;

    class JointBuffer {
        friend AnimationSystem;

    public:
        JointBuffer(JointRegistry* jointRegistry);
        JointBuffer& operator=(const JointBuffer&) = delete;

        ~JointBuffer();

        void clear();
        void prepare();

        void beginFrame();
        void endFrame();

    protected:
        void updateRT();

    private:
        void upload();

        void uploadSpan(
            const std::vector<JointTransformSSBO>& snapshot,
            const util::BufferReference& range);

        void resizeBuffer(size_t totalCount);

    private:
        JointRegistry* const m_jointRegistry;

        kigl::GLBuffer m_ssbo{ "joint_ssbo" };
        kigl::GLFence m_fence{ "joint_fence" };
        size_t m_entryCount{ 0 };

        size_t m_frameSkipCount{ 0 };
    };
}

#pragma once

#include <span>
#include <memory>

#include <glm/glm.hpp>

#include "kigl/kigl.h"

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "kigl/RingAllocator.h"

#include "gl/DrawIndirectCommand.h"
#include "gl/PerformanceCounters.h"

#include "MultiDrawRange.h"
#include "DrawOptions.h"

#include "render/InstanceIndexSSBO.h"

class Program;
struct PrepareContext;


namespace backend {
    struct DrawRange {
        uint32_t commandCount;
        MultiDrawRange params;
    };

    class DrawBuffer {
    public:
        DrawBuffer();

        void prepareRT();

        void bind();

        void beginFrame();
        void endFrame();

        // STEPS:
        // - sendInstanceIndeces
        // - send or sendDirect
        // - flush
        // - finish
        // @return true if sending succeeded (if not cannot do draws)
        bool sendInstanceIndeces(
            std::span<render::InstanceIndexSSBO> indeces);

        void send(
            const backend::MultiDrawRange& sendRange,
            const backend::gl::DrawIndirectCommand& cmd);

        void flush();

        gl::PerformanceCounters getCounters(bool clear) const;

    private:
        void flushIfNeeded();

        bool isSameMultiDraw(
            const backend::MultiDrawRange& sendRange);

        void drawPending();

        void bindMultiDrawRange(
            const backend::MultiDrawRange& drawRange) const;

    private:
        bool m_batchDebug{ false };

        bool m_bound = false;

        // Separate ring allocators for instances and draw commands
        std::unique_ptr<kigl::RingAllocator> m_instanceRing{ nullptr };
        std::unique_ptr<kigl::RingAllocator> m_commandRing{ nullptr };

        // Current frame allocations
        kigl::RingAllocation<render::InstanceIndexSSBO> m_currentInstanceAlloc{};
        kigl::RingAllocation<backend::gl::DrawIndirectCommand> m_currentCommandAlloc{};
        size_t m_commandCount = 0;
        size_t m_commandCapacity = 0;

        std::vector<DrawRange> m_drawRanges;

        size_t m_drawCounter = 0;
    };
}

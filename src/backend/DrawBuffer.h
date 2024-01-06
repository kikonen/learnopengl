#pragma once

#include <glm/glm.hpp>

#include "kigl/kigl.h"

#include "kigl/GLState.h"
#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "kigl/GLSyncQueue.h"

#include "gl/DrawIndirectCommand.h"
#include "gl/PerformanceCounters.h"

#include "DrawRange.h"
#include "DrawOptions.h"

class Assets;
class Program;
class Registry;

namespace backend {
    // WIP KI it seems that there was no corruption in NVidia even if sync is turned off
    using GLCommandQueue = kigl::GLSyncQueue<backend::gl::DrawIndirectCommand, true>;

    class DrawBuffer {
    public:
        DrawBuffer(
            bool useFence,
            bool useSingleFence);

        void prepareRT(
            const Assets& assets,
            Registry* registry,
            int batchCount,
            int rangeCount);

        void bind();

        void send(
            const backend::DrawRange& sendRange,
            const backend::gl::DrawIndirectCommand& cmd);

        void flushIfNeeded();
        void flush();

        void flushIfNotSame(
            const backend::DrawRange& sendRange);

        void drawPending(bool drawCurrent);

        gl::PerformanceCounters getCounters(bool clear) const;

    private:
        void bindDrawRange(
            const backend::DrawRange& drawRange) const;

    private:
        const bool m_useFence;
        const bool m_useSingleFence;

        int m_batchCount = 0;
        int m_rangeCount = 0;

        bool m_frustumGPU = false;

        bool m_bound = false;

        glm::uvec3 m_computeGroups{ 0 };

        Program* m_cullingCompute{ nullptr };

        std::unique_ptr<GLCommandQueue> m_commands{ nullptr };

        std::vector<backend::DrawRange> m_drawRanges;

        kigl::GLBuffer m_drawParameters{ "draw_parameters" };
        kigl::GLBuffer m_performanceCounters{ "draw_performance_counters" };

        size_t m_drawCounter = 0;
    };
}

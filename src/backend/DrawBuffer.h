#pragma once

#include <span>

#include <glm/glm.hpp>

#include "kigl/kigl.h"

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "kigl/GLSyncQueue.h"

#include "gl/DrawIndirectCommand.h"
#include "gl/PerformanceCounters.h"

#include "mesh/InstanceSSBO.h"

#include "MultiDrawRange.h"
#include "DrawOptions.h"

class Program;
struct PrepareContext;

namespace backend {
    // WIP KI it seems that there was no corruption in NVidia even if sync is turned off
    using GLCommandQueue = kigl::GLSyncQueue<backend::gl::DrawIndirectCommand>;

    class DrawBuffer {
    public:
        DrawBuffer(
            bool useMapped,
            bool useInvalidate,
            bool useFence,
            bool useSingleFence,
            bool useDebugFence);

        void prepareRT(
            const PrepareContext& ctx,
            int batchCount,
            int rangeCount);

        void bind();

        // STEPS:
        // - sendInstanceIndeces
        // - send or sendDirect
        // - flush
        // - drawPending
        void sendInstanceIndeces(
            std::span<mesh::InstanceSSBO> indeces);

        void send(
            const backend::MultiDrawRange& sendRange,
            const backend::gl::DrawIndirectCommand& cmd);

        void sendDirect(
            const backend::MultiDrawRange& sendRange,
            const backend::gl::DrawIndirectCommand& cmd);

        void flush();
        void drawPending(bool drawCurrent);

        gl::PerformanceCounters getCounters(bool clear) const;

    private:
        void flushIfNeeded();

        void flushIfNotSameMultiDraw(
            const backend::MultiDrawRange& sendRange);

        void bindMultiDrawRange(
            const backend::MultiDrawRange& drawRange) const;

    private:
        const bool m_useMapped;
        const bool m_useInvalidate;
        const bool m_useFence;
        const bool m_useSingleFence;
        const bool m_useDebugFence;

        int m_batchCount = 0;
        int m_rangeCount = 0;

        bool m_frustumGPU = false;

        bool m_bound = false;

        glm::uvec3 m_computeGroups{ 0 };

        ki::program_id m_cullingComputeId{ 0 };
        Program* m_cullingCompute{ nullptr };

        std::unique_ptr<GLCommandQueue> m_commands{ nullptr };

        std::vector<backend::MultiDrawRange> m_drawRanges;

        kigl::GLBuffer m_indexBuffer{ "instance_index" };

        kigl::GLBuffer m_drawParameters{ "draw_parameters" };
        kigl::GLBuffer m_performanceCounters{ "draw_performance_counters" };

        size_t m_drawCounter = 0;
    };
}

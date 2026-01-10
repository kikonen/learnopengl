#include "DrawBuffer.h"

#include <tuple>

#include <fmt/format.h>

#include "kigl/GLState.h"

#include "util/util.h"

#include "asset/Assets.h"

#include "shader/SSBO.h"
#include "shader/Program.h"
#include "shader/ProgramUniforms.h"
#include "shader/Shader.h"
#include "shader/uniform.h"
#include "shader/ProgramRegistry.h"

#include "engine/PrepareContext.h"

#include "registry/Registry.h"

#include "backend/gl/DrawIndirectParameters.h"
#include "backend/gl/PerformanceCounters.h"
#include "DrawOptions.h"

#include "render/InstanceIndexSSBO.h"

namespace {
    inline const std::string CS_FRUSTUM_CULLING{ "frustum_culling" };

    constexpr size_t INDEX_BLOCK_SIZE = 1000;
    constexpr size_t MAX_INDEX_BLOCK_COUNT = 500;

    constexpr size_t MAX_INDEX_COUNT = INDEX_BLOCK_SIZE * MAX_INDEX_BLOCK_COUNT;

    // Estimate sizes per frame
    constexpr size_t INSTANCES_PER_FRAME = 50000;
    constexpr size_t COMMANDS_PER_FRAME = 100;
    constexpr size_t BATCHES_PER_FRAME = 70;

    constexpr size_t estimateInstanceSizePerFrame()
    {
        return BATCHES_PER_FRAME * INSTANCES_PER_FRAME * sizeof(render::InstanceIndexSSBO)
            + 16 * 256;  // alignment headroom
    }

    constexpr size_t estimateCommandSizePerFrame()
    {
        return BATCHES_PER_FRAME * COMMANDS_PER_FRAME * sizeof(backend::gl::DrawIndirectCommand)
            + 16 * 256;  // alignment headroom
    }
}

namespace backend {
    DrawBuffer::DrawBuffer()
    {
    }

    void DrawBuffer::prepareRT()
    {
        const auto& assets = Assets::get();

        m_batchDebug = assets.batchDebug;

        // Create instance ring allocator
        {
            GLint bufferAlignment;
            glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &bufferAlignment);

            m_instanceRing = std::make_unique<kigl::RingAllocator>(
                "draw_instance_ring",
                bufferAlignment,
                3,
                1.5f);
            m_instanceRing->create(estimateInstanceSizePerFrame());
        }

        // Create command ring allocator
        {
            constexpr size_t BUFFER_ALIGNMENT = 4;

            m_commandRing = std::make_unique<kigl::RingAllocator>(
                "draw_command_ring",
                BUFFER_ALIGNMENT,
                3,
                1.5f);
            m_commandRing->create(estimateCommandSizePerFrame());
        }
    }

    void DrawBuffer::beginFrame()
    {
        m_instanceRing->beginFrame();
        m_commandRing->beginFrame();

        // Pre-allocate command buffer for this frame
        m_commandCapacity = COMMANDS_PER_FRAME;
        m_currentCommandAlloc = m_commandRing->allocate<backend::gl::DrawIndirectCommand>(m_commandCapacity);
        m_commandCount = 0;
    }

    void DrawBuffer::endFrame()
    {
        m_instanceRing->endFrame();
        m_commandRing->endFrame();

        m_currentInstanceAlloc = {};
        m_currentCommandAlloc = {};
        m_commandCount = 0;
        m_commandCapacity = 0;
    }

    void DrawBuffer::bind()
    {
        if (m_bound) return;
        m_bound = true;
    }

    void DrawBuffer::flushIfNeeded()
    {
        if (m_commandCount < m_commandCapacity) return;
        flush();
    }

    void DrawBuffer::flush()
    {
        const int drawCount = static_cast<int>(m_commandCount);

        if (drawCount == 0) return;

        m_drawCounter += drawCount;

        drawPending();

        // Allocate new command buffer for next batch
        m_currentCommandAlloc = m_commandRing->allocate<backend::gl::DrawIndirectCommand>(m_commandCapacity);
        m_commandCount = 0;
    }

    bool DrawBuffer::isSameMultiDraw(
        const backend::MultiDrawRange& sendRange)
    {
        if (m_commandCount == 0) return false;

        auto& curr = m_drawRanges.back().params;

        const auto& cd = curr.m_drawOptions;
        const auto& sd = sendRange.m_drawOptions;

        // NOTE KI KIND_SOLID & KIND_ALPHA can be in same multidraw
        return curr.m_vaoId == sendRange.m_vaoId &&
            curr.m_programId == sendRange.m_programId &&
            cd.m_renderBack == sd.m_renderBack &&
            cd.m_lineMode == sd.m_lineMode &&
            cd.isBlend() == sd.isBlend() &&
            cd.m_mode == sd.m_mode &&
            cd.m_type == sd.m_type;
    }

    void DrawBuffer::send(
        const backend::MultiDrawRange& sendRange,
        const backend::gl::DrawIndirectCommand& cmd)
    {
        if (isSameMultiDraw(sendRange)) {
            m_drawRanges.back().commandCount++;
        }
        else {
            // starting new range
            m_drawRanges.push_back({ .commandCount = 1, .params = sendRange });
        }

        // Write command to ring buffer
        if (m_currentCommandAlloc && m_commandCount < m_commandCapacity) {
            m_currentCommandAlloc[m_commandCount] = cmd;
            m_commandCount++;

            if (m_commandCount >= m_commandCapacity) {
                flush();
            }
        }
    }

    void DrawBuffer::sendInstanceIndeces(
        std::span<render::InstanceIndexSSBO> indeces)
    {
        const size_t totalCount = indeces.size();

        m_currentInstanceAlloc = m_instanceRing->allocate<render::InstanceIndexSSBO>(totalCount);

        if (m_currentInstanceAlloc) {
            std::copy(
                std::begin(indeces),
                std::end(indeces),
                m_currentInstanceAlloc.data);

            m_instanceRing->bindSSBO(SSBO_INSTANCE_INDECES, m_currentInstanceAlloc.ref);
        }
    }

    void DrawBuffer::drawPending()
    {
        m_commandRing->bindDrawIndirect();

        size_t tallyCount = 0;
        constexpr auto sz = sizeof(backend::gl::DrawIndirectCommand);

        for (const auto& drawEntry : m_drawRanges) {
            const auto& drawRange = drawEntry.params;
            const auto& drawCount = drawEntry.commandCount;
            const auto baseOffset = m_currentCommandAlloc.ref.offset + tallyCount * sz;

            const auto& drawOptions = drawRange.m_drawOptions;

            bindMultiDrawRange(drawRange);

            if (drawOptions.m_type == backend::DrawOptions::Type::elements) {
                glMultiDrawElementsIndirect(
                    drawOptions.toMode(),
                    GL_UNSIGNED_INT,
                    (void*)baseOffset,
                    drawCount,
                    sz);
            }
            else if (drawOptions.m_type == backend::DrawOptions::Type::arrays)
            {
                glMultiDrawArraysIndirect(
                    drawOptions.toMode(),
                    (void*)baseOffset,
                    drawCount,
                    sz);
            }
            tallyCount += drawCount;
        }

        m_drawRanges.clear();
    }

    gl::PerformanceCounters DrawBuffer::getCounters(bool clear) const
    {
        gl::PerformanceCounters counters;
        return counters;
    }

    void DrawBuffer::bindMultiDrawRange(
        const backend::MultiDrawRange& drawRange) const
    {
        auto& state = kigl::GLState::get();
        const auto& drawOptions = drawRange.m_drawOptions;

        // NOTE KI bind vao only if used for this draw
        if (drawRange.m_vaoId) {
            state.bindVAO(drawRange.m_vaoId);
        }

        auto* program = Program::get(drawRange.m_programId);
        state.useProgram(*program);

        state.setEnabled(GL_CULL_FACE, !drawOptions.m_renderBack);

        const bool lineMode = drawOptions.m_lineMode;

        if (drawOptions.m_reverseFrontFace) {
            state.frontFace(GL_CW);
        }
        else {
            state.frontFace(GL_CCW);
        }

        if (drawOptions.m_noDepth) {
            state.setEnabled(GL_DEPTH_TEST, false);
        }
        else {
            state.setEnabled(GL_DEPTH_TEST, true);
        }

        state.polygonFrontAndBack(lineMode ? GL_LINE : GL_FILL);

        if (drawOptions.m_mode == DrawOptions::Mode::patches) {
            glPatchParameteri(GL_PATCH_VERTICES, drawOptions.m_patchVertices);
        }

        const bool blend = !lineMode && drawOptions.isBlend();
        state.setEnabled(GL_BLEND, blend);
        if (blend) {
            // NOTE KI no blend mode with OIT blend
            if (!drawOptions.m_gbuffer) {
                state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });
            }
        }

        state.setEnabled(GL_CLIP_DISTANCE1, drawRange.m_drawOptions.m_clip);
    }
}

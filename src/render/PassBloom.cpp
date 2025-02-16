#include "PassBloom.h"

#include "kigl/GLState.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"
#include "shader/ProgramUniforms.h"

#include "mesh/MeshType.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/NodeDraw.h"
#include "render/Batch.h"

namespace {
}

namespace render
{
    PassBloom::PassBloom()
        : Pass("PassBloom")
    {
    }

    PassBloom::~PassBloom() = default;

    void PassBloom::prepare(const PrepareContext& ctx)
    {
        m_bloomInitProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_BLOOM_INIT_PASS));
        //m_bloomBlurProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_BLOOM_BLUR_PASS));
        //m_bloomFinalProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_BLOOM_FINAL_PASS));
        m_blurVerticalProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_BLUR_VERTICAL));
        m_blurHorizontalProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_BLUR_HORIZONTAL));
        m_blurFinalProgramCS = Program::get(ProgramRegistry::get().getComputeProgram(CS_BLUR_FINAL, {}));

        m_blurBuffer.prepare();
    }

    void PassBloom::updateRT(const UpdateViewContext& ctx, float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;

        m_blurBuffer.updateRT(ctx, bufferScale);
    }

    void PassBloom::initRender(const RenderContext& ctx)
    {
        const auto& dbg = *ctx.m_dbg;

        m_enabled = !ctx.m_forceSolid
            && ctx.m_useBloom
            && dbg.m_effectBloomEnabled;

        if (m_enabled) {
            for (auto& buffer : m_blurBuffer.m_buffers) {
                buffer->clearAll();
            }
        }
    }

    PassContext PassBloom::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        startScreenPass(
            ctx,
            false,
            {},
            false,
            {});

        passBloom(ctx, drawContext, src);

        stopScreenPass(ctx);

        return src;
    }

    void PassBloom::passBloom(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        auto& state = ctx.m_state;

        FrameBuffer* prev = nullptr;
        for (int i = 0; i < BlurBuffer::BUFFER_COUNT; i++)
        {
            auto* buffer = m_blurBuffer.m_buffers[i].get();
            buffer->bind(ctx);

            {
                if (prev) {
                    prev->bindTexture(ctx, BlurBuffer::ATT_COLOR_B_INDEX, UNIT_SOURCE);
                    m_blurHorizontalProgram->bind();
                }
                else {
                    // NOTE KI for first step, use *original* as source
                    // => And do init pass of collecting bright values
                    src.buffer->bindTexture(ctx, src.attachmentIndex, UNIT_SOURCE);
                    m_bloomInitProgram->bind();
                }

                buffer->setDrawBuffer(BlurBuffer::ATT_COLOR_A_INDEX);

                m_screenTri.draw();
            }

            {
                buffer->bindTexture(ctx, BlurBuffer::ATT_COLOR_A_INDEX, UNIT_SOURCE);
                buffer->setDrawBuffer(BlurBuffer::ATT_COLOR_B_INDEX);

                m_blurVerticalProgram->bind();
                m_screenTri.draw();
            }

            prev = buffer;
        }

        {
            const int channels[BlurBuffer::BUFFER_COUNT] = {
                UNIT_CHANNEL_0,
                UNIT_CHANNEL_1,
                UNIT_CHANNEL_2,
                //UNIT_CHANNEL_3,
            };

            for (int i = 0; i < BlurBuffer::BUFFER_COUNT; i++) {
                auto* buffer = m_blurBuffer.m_buffers[i].get();
                buffer->bindTexture(ctx, BlurBuffer::ATT_COLOR_B_INDEX, channels[i]);
            }

            //{
            //    auto* buffer = m_blurBuffer.m_buffers[0].get();
            //    buffer->bindTexture(ctx, BlurBuffer::ATT_COLOR_A_INDEX, UNIT_SOURCE);
            //}

            {
                if (!m_blurFinalProgramCS->isReady()) return;

                // NOTE KI image textures cannot be bound into high units for some reason
                src.buffer->bindImageTexture(ctx, 0, UNIT_0);

                m_blurFinalProgramCS->bind();

                const auto& viewport = src.buffer->getSize();
                auto* uniforms = m_blurFinalProgramCS->m_uniforms.get();
                uniforms->u_viewport.set(viewport);

                if (false) {
                    // NOTE KI need to process also non-aligned portion of viewport
                    int groupX = viewport.x / 15 + (viewport.x % 15 != 0 ? 1 : 0);
                    int groupY = viewport.y / 4 + (viewport.y % 4 != 0 ? 1 : 0);

                    glDispatchCompute(groupX, groupY, 1);
                }
                else {
                    glDispatchCompute(viewport.x, viewport.y, 1);
                }

                //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
        }
    }

    //void PassBloom::passBloom(
    //    const RenderContext& ctx,
    //    const DrawContext& drawContext)
    //{
    //    auto& state = ctx.m_state;

    //    if (!m_effectBloomEnabled || !ctx.m_useBloom)
    //    {
    //        srcBuffer->copy(
    //            finalBuffer,
    //            EffectBuffer::ATT_ALBEDO_INDEX,
    //            EffectBuffer::ATT_ALBEDO_INDEX);
    //        return;
    //    }

    //    for (auto& buffer : m_effectBuffer.m_buffers) {
    //        buffer->clearAll();
    //    }

    //    {
    //        srcBuffer->bindTexture(ctx, EffectBuffer::ATT_ALBEDO_INDEX, UNIT_EFFECT_ALBEDO);

    //        auto& buf = m_effectBuffer.m_buffers[1];
    //        buf->bind(ctx);
    //        buf->bindTexture(ctx, EffectBuffer::ATT_WORK_INDEX, UNIT_EFFECT_WORK);

    //        m_bloomInitProgram->bind();
    //        m_screenTri.draw();
    //    }

    //    {
    //        //srcBuffer->bindTexture(ctx, EffectBuffer::ATT_ALBEDO_INDEX, UNIT_EFFECT_ALBEDO);
    //        //srcBuffer->bindTexture(ctx, EffectBuffer::ATT_BRIGHT_INDEX, UNIT_EFFECT_BRIGHT);

    //        //m_emissionProgram->bind();
    //        //m_screenTri.draw(ctx);
    //    }

    //    m_bloomBlurProgram->bind();

    //    bool horizontal = false;
    //    for (int i = 0; i < m_effectBloomIterations; i++) {
    //        auto& buf = m_effectBuffer.m_buffers[i % 2];
    //        buf->bind(ctx);

    //        m_bloomBlurProgram->m_uniforms->u_effectBloomHorizontal.set(horizontal);
    //        m_screenTri.draw();

    //        buf->bindTexture(ctx, EffectBuffer::ATT_WORK_INDEX, UNIT_EFFECT_WORK);
    //        horizontal = !horizontal;
    //    }

    //    {
    //        finalBuffer->clearAll();
    //        finalBuffer->bind(ctx);

    //        srcBuffer->bindTexture(ctx, EffectBuffer::ATT_ALBEDO_INDEX, UNIT_EFFECT_ALBEDO);

    //        m_bloomFinalProgram->bind();
    //        m_screenTri.draw();
    //    }
    //}
}

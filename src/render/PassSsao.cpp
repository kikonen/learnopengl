#include "PassSsao.h"

#include <random>

#include "kigl/GLState.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"

#include "SsaoBuffer.h"

namespace {
    const std::string SHADER_SSAO_PASS{ "screen_ssao_pass" };
    const std::string SHADER_SSAO_BLUR_PASS{ "screen_ssao_blur_pass" };
    const std::string U_SAMPLES{ "u_samples" };

    std::vector<glm::vec3> s_kernelValues;
    std::vector<glm::vec3> s_noiseValues;

    // generate sample kernel
    // ----------------------
    void initKernel(std::vector<glm::vec3>& kernelValues)
    {
        // generates random floats between 0.0 and 1.0
        std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
        std::default_random_engine generator;

        kernelValues.reserve(64);

        for (unsigned int i = 0; i < 64; ++i)
        {
            glm::vec3 sample(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator));

            sample = glm::normalize(sample);
            sample *= randomFloats(generator);
            float scale = float(i) / 64.0f;

            // scale samples s.t. they're more aligned to center of kernel
            scale = glm::lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            kernelValues.push_back(sample);
        }
    }

    // generate noise texture
    // ----------------------
    void initNoise(std::vector<glm::vec3>& noiseValues)
    {
        std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
        std::default_random_engine generator;

        noiseValues.reserve(16);

        for (unsigned int i = 0; i < 16; i++)
        {
            // rotate around z-axis (in tangent space)
            glm::vec3 noise(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                0.0f);
            noiseValues.push_back(noise);
        }
    }

    const std::vector<glm::vec3>& getKernelValues()
    {
        if (s_kernelValues.empty())
        {
            initKernel(s_kernelValues);
        }
        return s_kernelValues;
    }

    const std::vector<glm::vec3>& getNoiseValues()
    {
        if (s_noiseValues.empty())
        {
            initNoise(s_noiseValues);
        }
        return s_noiseValues;
    }
}

namespace noise {
    struct NoiseTexture {
        int m_usageCount{ 0 };
        bool m_prepared{ false };
        kigl::GLTextureHandle m_tex;

        void prepare() {
            if (m_prepared) return;
            m_prepared = true;

            constexpr int w = 4;
            constexpr int h = 4;
            m_tex.create("ssao_noise_tex", GL_TEXTURE_2D, w, h);
            int texId = m_tex;

            glTextureParameteri(texId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(texId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glTextureParameteri(texId, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(texId, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTextureStorage2D(texId, 1, GL_RGBA16F, w, h);
            glTextureSubImage2D(texId, 0, 0, 0, w, h, GL_RGB, GL_FLOAT, getNoiseValues().data());
            glGenerateTextureMipmap(texId);
        }
    };

    std::unique_ptr<NoiseTexture> m_noiseTex;

    void reserveNoiseTexture() {
        if (!m_noiseTex) {
            m_noiseTex = std::make_unique<NoiseTexture>();
            m_noiseTex->prepare();
        }
        m_noiseTex->m_usageCount++;
    }

    void releaseNoiseTexture() {
        if (m_noiseTex) {
            m_noiseTex->m_usageCount--;
            if (m_noiseTex->m_usageCount <= 0) {
                m_noiseTex = nullptr;
            }
        }
    }

    NoiseTexture* getNoiseTexture()
    {
        return m_noiseTex.get();
    }
}

namespace render
{
    PassSsao::PassSsao()
        : Pass("PassSsao")
    {
        noise::reserveNoiseTexture();
    }

    PassSsao::~PassSsao()
    {
        noise::releaseNoiseTexture();
    }

    void PassSsao::prepare(const PrepareContext& ctx)
    {
        m_ssaoProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_SSAO_PASS));
        m_ssaoBlurProgram = Program::get(ProgramRegistry::get().getProgram(SHADER_SSAO_BLUR_PASS));

        noise::getNoiseTexture()->prepare();

        m_ssaoBuffer.prepare();
    }

    void PassSsao::updateRT(
        const UpdateViewContext& ctx,
        const std::string& namePrefix,
        float bufferScale)
    {
        if (!updateSize(ctx, bufferScale)) return;

        m_ssaoBuffer.updateRT(ctx, namePrefix, bufferScale);
    }

    void PassSsao::cleanup(const RenderContext& ctx)
    {
        if (!m_enabled) return;

        m_ssaoBuffer.invalidateAll();
    }

    void PassSsao::initRender(const RenderContext& ctx)
    {
        auto& state = ctx.m_state;
        const auto& dbg = ctx.m_dbg;

        m_enabled = !(ctx.m_forceSolid || !ctx.m_useScreenspaceEffects) &&
            ctx.m_useSsao &&
            dbg.m_effectSsaoEnabled;
    }

    PassContext PassSsao::render(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const PassContext& src)
    {
        if (!m_enabled) return src;

        auto& state = ctx.m_state;

        startScreenPass(
            ctx,
            false,
            {},
            false,
            {});

        m_ssaoBuffer.bind(ctx);

        //m_ssaoProgram->setVec3Array(U_SAMPLES, getKernelValues());
        m_ssaoProgram->bind();
        m_ssaoBuffer.m_buffer->setDrawBuffer(SsaoBuffer::ATT_SSAO_INDEX);
        m_ssaoBuffer.m_buffer->clearAttachment(SsaoBuffer::ATT_SSAO_INDEX);
        noise::getNoiseTexture()->m_tex.bindTexture(UNIT_NOISE);
        m_screenTri.draw();

        m_ssaoBlurProgram->bind();
        m_ssaoBuffer.bindSsaoTexture(state);
        m_ssaoBuffer.m_buffer->setDrawBuffer(SsaoBuffer::ATT_SSAO_BLUR_INDEX);
        m_ssaoBuffer.m_buffer->clearAttachment(SsaoBuffer::ATT_SSAO_BLUR_INDEX);
        m_screenTri.draw();

        m_ssaoBuffer.bindSsaoBlurTexture(state);

        stopScreenPass(ctx);

        return src;
    }

    const std::vector<glm::vec3>& PassSsao::getKernel() {
        return getKernelValues();
    }
}

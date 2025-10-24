#include "Engine.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <regex>

#include <fmt/format.h>

#include "imgui.h"

#include "util/util.h"
#include "util/Log.h"

#include "asset/Assets.h"

#include "ki/Timer.h"

#include "kigl/kigl.h"
#include "kigl/GLState.h"
#include "kigl/OpenGLInfo.h"

#include "material/MaterialSSBO.h"

#include "engine/AsyncLoader.h"
#include "engine/AssetsLoader.h"
#include "engine/InputContext.h"
#include "engine/PrepareContext.h"
#include "engine/UpdateViewContext.h"

#include "shader/ProgramRegistry.h"

#include "scene/Scene.h"

#include "render/Batch.h"
#include "render/RenderData.h"
#include "render/PassSsao.h"
#include "render/WindowBuffer.h"

#include "registry/VaoRegistry.h"
#include "registry/SelectionRegistry.h"

#include "gui/Window.h"

#include "SystemInit.h"

#include "UpdateContext.h"

namespace
{
}

Engine::Engine()
    : m_alive(std::make_shared<std::atomic_bool>(true)),
    m_dbg{ debug::DebugContext::modify() }
{
}

Engine::~Engine() {
    *m_alive = false;
}

int Engine::init()
{
    Assets::set(loadAssets());

    SystemInit::init();
    onInit();

    m_registry = std::make_unique<Registry>(*this, m_alive);

    m_asyncLoader = std::make_shared<AsyncLoader>();

    m_window = std::make_unique<Window>(*this);
    return m_window->create() ? 0 : -1;
}

int Engine::setup() {
    const auto& assets = Assets::get();
    auto& state = kigl::GLState::get();

    const GLenum keys[] = {
        GL_BLEND,
        GL_CLIP_DISTANCE0,
        GL_CLIP_DISTANCE1,
        GL_CLIP_DISTANCE2,
        GL_CLIP_DISTANCE3,
        GL_CULL_FACE,
        GL_DEPTH_TEST,
        GL_POLYGON_OFFSET_FILL,
        GL_PROGRAM_POINT_SIZE,
        GL_RASTERIZER_DISCARD,
        GL_STENCIL_TEST,
        GL_TEXTURE_CUBE_MAP_SEAMLESS,
    };
    for (auto& key : keys) {
        state.track(key);
    }

    m_registry->prepare({ *this });

    m_batch = std::make_unique<render::Batch>();
    m_batch->prepareRT({ *this });

    m_renderData = std::make_unique<render::RenderData>();
    m_renderData->prepare(
        false,
        assets.glUseInvalidate,
        assets.glUseFence,
        assets.glUseFenceDebug,
        assets.batchDebug);

    {
        m_windowBuffer = std::make_unique<render::WindowBuffer>(true);
    }

    return onSetup();
}

int Engine::update()
{
    UpdateContext ctx{ *this, m_clock };

    // NOTE KI race condition with program prepare and event processing
    // NOTE KI also race with snapshot and event processing
    // => doing programs before snapshot reduce scope
    //    but DOES NOT remove it
    ProgramRegistry::get().updateRT(ctx);

    getRegistry()->m_dispatcherView->dispatchEvents();

    {
        const glm::ivec2& size = getSize();
        UpdateViewContext updateCtx{
            *this,
            size.x,
            size.y };
        m_windowBuffer->updateRT(updateCtx);
    }

    m_batch->updateRT(ctx);
    return onUpdate(ctx);
}

int Engine::render()
{
    prepareUBOs();
    updateUBOs();

    VaoRegistry::get().bindDefaultVao();

    m_batch->bind();
    int result = onRender(m_clock);
    getRenderData()->invalidateAll();
    return result;
}

void Engine::processInput()
{
    InputContext ctx{
        *this,
        *m_window->m_input.get() };

    // NOTE KI input after checkin imgui focus
    m_window->processInput(ctx);
}


void Engine::run() {
    auto& assets = Assets::modify();

    {
        const auto& info = kigl::GL::getInfo();
        info.dumpInfo();

        assets.glVendorNvidia = info.isNvidia();
        assets.glVendorIntel = info.isIntel();

        assets.glPreferredTextureFormatRGBA = info.m_preferredFormatRGBA8;
        assets.glPreferredTextureFormatRGB = info.m_preferredFormatRGB8;
    }

    KI_INFO("setup");
    if (!assets.glNoError) {
        kigl::GL::startError();
    }

    if (assets.glDebug) {
        // NOTE KI MUST AFTER glfwWindow create
        kigl::GL::startDebug();
    }

    int res = setup();
    if (res) {
        m_window->close();
    }

    auto prevLoopStart = std::chrono::high_resolution_clock::now();
    auto loopStart = std::chrono::high_resolution_clock::now();
    auto loopEnd = std::chrono::high_resolution_clock::now();

    std::chrono::duration<float> elapsedDuration;

    auto& clock = m_clock;
    auto& fpsCounter = m_fpsCounter;

    // render loop
    // -----------
    while (!m_window->isClosed())
    {
        // Calculate the target time per frame
        const double TARGET_FRAME_RATE = m_dbg.m_targetFrameRate;
        const double TARGET_MS_PER_FRAME = 1000.0 / (TARGET_FRAME_RATE * 1.5);

        fpsCounter.startFrame();

        int close = 0;

        {
            //KI_TIMER("loop");
            loopStart = std::chrono::high_resolution_clock::now();
            elapsedDuration = loopStart - prevLoopStart;

            clock.frameCount += 1;
            //auto ts = std::chrono::duration_cast<std::chrono::microseconds>(
            //    std::chrono::high_resolution_clock::now().time_since_epoch()
            //);
            //clock.ts = static_cast<double>(ts.count()) / (1000.0 * 1000.0);
            clock.ts = static_cast<double>(glfwGetTime());
            clock.elapsedSecs = elapsedDuration.count();

            // render
            // ------
            {
                fpsCounter.startRender();

                // serious sync issue entity data vs. drawing
                // - looks like camera is jerky, but it's lack of sync between
                //   draw loop and 3 of UBOs & mapped buffers in next frame
                // => INEFFICIENT, need to improve this
                // https://forums.developer.nvidia.com/t/persistent-buffer-synchronization-doesnt-work/66636/5
                if (assets.glUseFinish) {
                    //std::cout << ".";
                    glFinish();
                }

                if (!close) {
                    close = update();
                }
                if (!close) {
                    close = render();
                }
                {
                    processInput();
                }
                if (!close) {
                    UpdateContext ctx{ *this, m_clock };
                    close = onPost(ctx);
                }
                if (!close) {
                    ProgramRegistry::get().validate();
                }

                fpsCounter.endRender();
            }

            if (close) {
                m_window->close();
            }
        }


        if (!close) {
            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(m_window->m_glfwWindow);
        }

        if (!close) {
            prevLoopStart = loopStart;

            if (fpsCounter.isUpdate())
            {
                showFps(fpsCounter);
            }
        }

        fpsCounter.endFrame();
        //KI_GL_CHECK("engine.loop");

        {
            loopEnd = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(loopEnd - loopStart).count();
            if (duration < TARGET_MS_PER_FRAME) {
                //KI_INFO_OUT(fmt::format("wait: {}", TARGET_MS_PER_FRAME - duration));
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(TARGET_MS_PER_FRAME - duration)));
            }
        }

        glfwPollEvents();
    }

    onDestroy();
}

void Engine::showFps(const ki::FpsCounter& fpsCounter)
{
    auto& assets = Assets::modify();

    std::string title;
    if (auto* scene = m_currentScene.get(); scene)
    {
        title = fmt::format(
            "{}{}",
            scene->getName(), scene->isLoaded() ? " (LOADED)" : " loading...");
    }
    else {
        title = "OpenGL";
    }

    auto summary = fpsCounter.formatSummary(title.c_str());
    m_window->setTitle(summary);

    //if (m_window->isFullScreen() && !assets.glVendorNvidia) {
    //    std::cout << summary << '\n';
    //}
}

void Engine::onDestroy()
{
    SystemInit::release();

    *m_alive = false;
}

void Engine::prepareUBOs()
{
    //KI_INFO_OUT(fmt::format("ts: {}", m_data.u_time));
    const debug::DebugContext& dbg = m_dbg;
    const auto& assets = Assets::get();
    const auto& selectionRegistry = *getRegistry()->m_selectionRegistry;

    //auto cubeMapEnabled = dbg.m_cubeMapEnabled &&
    //    m_cubeMapRenderer->isEnabled() &&
    //    m_cubeMapRenderer->isRendered();

    m_dataUBO = {
        dbg.m_fogColor,
        // NOTE KI keep original screen resolution across the board
        // => current buffer resolution is separately in bufferInfo UBO
        //m_parent ? m_parent->m_resolution : m_resolution,

        selectionRegistry.getSelectionMaterialIndex(),
        selectionRegistry.getTagMaterialIndex(),

        dbg.m_cubeMapEnabled,
        assets.skyboxEnabled,

        assets.environmentMapEnabled,

        dbg.m_shadowVisual,
        dbg.m_forceLineMode,

        dbg.m_fogStart,
        dbg.m_fogEnd,
        dbg.m_fogDensity,

        dbg.m_effectOitMinBlendThreshold,
        dbg.m_effectOitMaxBlendThreshold,

        dbg.m_effectBloomThreshold,

        dbg.m_gammaCorrect,
        dbg.m_hdrExposure,

        static_cast<float>(getClock().ts),
        static_cast<int>(getClock().frameCount),
    };

    for (int i = 0; const auto& v : render::PassSsao::getKernel()) {
        if (i >= 64) break;
        m_dataUBO.u_ssaoSamples[i++] = v;
    }

    {
        float parallaxDepth = -1.f;
        if (!dbg.m_parallaxEnabled) {
            parallaxDepth = 0;
        }
        else if (dbg.m_parallaxDebugEnabled) {
            parallaxDepth = dbg.m_parallaxDebugDepth;
        }

        m_debugUBO = {
            dbg.m_wireframeLineColor,
            dbg.m_skyboxColor,
            dbg.m_effectSsaoBaseColor,

            dbg.m_wireframeOnly,
            dbg.m_wireframeLineWidth,

            dbg.m_entityId,
            dbg.m_animation.m_jointIndex,
            dbg.m_animation.m_debugJointWeight,

            dbg.m_lightEnabled,
            dbg.m_normalMapEnabled,

            dbg.m_skyboxColorEnabled,

            dbg.m_effectSsaoEnabled,
            dbg.m_effectSsaoBaseColorEnabled,

            parallaxDepth,
            dbg.m_parallaxMethod,
        };
    }
}

void Engine::updateUBOs() const
{
    // https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs
    auto* renderData = getRenderData();
    renderData->updateData(m_dataUBO);
    renderData->updateDebug(m_debugUBO);
}

const glm::ivec2& Engine::getSize() const
{
    return m_window->getSize();
}

backend::gl::PerformanceCounters Engine::getCounters(bool clear) const
{
    return m_batch->getCounters(clear);
}

backend::gl::PerformanceCounters Engine::getCountersLocal(bool clear) const
{
    return m_batch->getCountersLocal(clear);
}

Assets Engine::loadAssets()
{
    AssetsLoader loader{ "scene/assets.yml" };
    return loader.load();
}

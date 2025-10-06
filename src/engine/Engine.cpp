#include "Engine.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <regex>

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

#include "shader/ProgramRegistry.h"

#include "scene/Scene.h"

#include "render/Batch.h"
#include "render/RenderData.h"
#include "render/PassSsao.h"

#include "registry/VaoRegistry.h"
#include "registry/SelectionRegistry.h"

#include "gui/Window.h"

#include "SystemInit.h"

#include "UpdateContext.h"


Engine::Engine()
    : m_alive(std::make_shared<std::atomic<bool>>(true)),
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

    m_registry->prepareShared();
    m_registry->prepareRT({ *this });

    m_batch = std::make_unique<render::Batch>();
    m_batch->prepareRT({ *this });

    m_renderData = std::make_unique<render::RenderData>();
    m_renderData->prepare(
        false,
        assets.glUseInvalidate,
        assets.glUseFence,
        assets.glUseFenceDebug,
        assets.batchDebug);

    return onSetup();
}

int Engine::update()
{
    UpdateContext ctx{ *this };

    // NOTE KI race condition with program prepare and event processing
    // NOTE KI also race with snapshot and event processing
    // => doing programs before snapshot reduce scope
    //    but DOES NOT remove it
    ProgramRegistry::get().updateRT(ctx);

    getRegistry()->m_dispatcherView->dispatchEvents();

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

    auto prevLoopTime = std::chrono::system_clock::now();
    auto loopTime = std::chrono::system_clock::now();

    std::chrono::duration<float> elapsedDuration;

    auto& clock = m_clock;
    auto& fpsCounter = m_fpsCounter;

    // render loop
    // -----------
    while (!m_window->isClosed())
    {
        int close = 0;

        {
            //KI_TIMER("loop");
            loopTime = std::chrono::system_clock::now();
            elapsedDuration = loopTime - prevLoopTime;

            clock.frameCount += 1;
            //auto ts = std::chrono::duration_cast<std::chrono::microseconds>(
            //    std::chrono::system_clock::now().time_since_epoch()
            //);
            //clock.ts = static_cast<double>(ts.count()) / (1000.0 * 1000.0);
            clock.ts = static_cast<double>(glfwGetTime());
            clock.elapsedSecs = elapsedDuration.count();

            //if (m_registry->m_pendingSnapshotRegistry->isDirty()) {
            //    //KI_INFO("COPY: snapshot_dirty");
            //    m_registry->m_pendingSnapshotRegistry->copyTo(
            //        m_registry->m_activeSnapshotRegistry,
            //        0, -1);
            ////}
            ////else {
            ////    KI_INFO("SKIP: snapshot_not_dirty");
            //}

            // render
            // ------
            {
                fpsCounter.startFame();

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
                    UpdateContext ctx{ *this };
                    close = onPost(ctx);
                }
                if (!close) {
                    ProgramRegistry::get().validate();
                }

                fpsCounter.endFame(clock.elapsedSecs);
            }

            if (close) {
                m_window->close();
            }
        }


        if (!close) {
            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(m_window->m_glfwWindow);
            glfwPollEvents();
        }

        if (!close) {
            prevLoopTime = loopTime;

            if (fpsCounter.isUpdate())
            {
                showFps(fpsCounter);
            }
        }

        //KI_GL_CHECK("engine.loop");
    }

    onDestroy();
}

void Engine::showFps(const ki::FpsCounter& fpsCounter)
{
    auto& assets = Assets::modify();

    std::string title = m_currentScene ? m_currentScene->getName() : "OpenGL";

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
            dbg.m_animation.m_boneIndex,
            dbg.m_animation.m_debugBoneWeight,

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

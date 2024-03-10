#include "Engine.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <regex>

#include "imgui.h"

#include "util/Util.h"
#include "util/Log.h"

#include "asset/Assets.h"

#include "ki/FpsCounter.h"
#include "ki/Timer.h"

#include "kigl/kigl.h"
#include "kigl/GLState.h"
#include "kigl/OpenGLInfo.h"

#include "asset/MaterialSSBO.h"

#include "engine/AsyncLoader.h"
#include "engine/InputContext.h"

#include "registry/ProgramRegistry.h"
#include "registry/NodeSnapshotRegistry.h"

#include "registry/SnapshotRegistry_impl.h"

#include "gui/Window.h"


Engine::Engine()
    : m_alive(std::make_shared<std::atomic<bool>>(true))
{
}

Engine::~Engine() {
    *m_alive = false;
}

int Engine::init() {

    onInit();

    m_registry = std::make_shared<Registry>(m_alive);
    m_asyncLoader = std::make_shared<AsyncLoader>(m_alive);

    m_window = std::make_unique<Window>(*this);
    return m_window->create() ? 0 : -1;
}

int Engine::setup() {
    auto& state = kigl::GLState::get();

    const GLenum keys[] = {
        GL_BLEND,
        GL_CLIP_DISTANCE0,
        GL_CLIP_DISTANCE1,
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
    m_registry->prepareRT();

    return onSetup();
}

void Engine::run() {
    auto& assets = Assets::modify();

    const auto& info = kigl::GL::getInfo();
    const auto& extensions = kigl::GL::getExtensions();
    // NOTE KI https://www.khronos.org/opengl/wiki/Common_Mistakes
    // - preferredFormat is performnce topic
    KI_INFO_OUT(fmt::format(
R"(
ENGINE::RUN
-------------
vendor:   {}
renderer: {}
version:  {}
glsl:     {}
-------------
GL_MAX_VERTEX_UNIFORM_COMPONENTS:  {}
GL_MAX_VERTEX_ATTRIBS:             {}
GL_MAX_COMPUTE_WORK_GROUP_COUNT:   {}
GL_PREFERRED_TEXTURE_FORMAT_RGBA8: 0x{:x}
GL_PREFERRED_TEXTURE_FORMAT_RGB8:  0x{:x}
)",
        info.vendor,
        info.renderer,
        info.version,
        info.glslVersion,
        info.maxVertexUniformComponents,
        info.maxVertexAttributes,
        info.formatMaxComputeWorkGroupCount(),
        info.preferredFormatRGBA8,
        info.preferredFormatRGB8));

    const auto vendor = util::toLower(info.vendor);
    assets.glVendorNvidia = std::regex_match(vendor, std::regex(".*nvidia.*"));
    assets.glVendorIntel = std::regex_match(vendor, std::regex(".*intel.*"));

    KI_INFO("[EXTENSIONS]");
    for (const auto& ext : extensions) {
        KI_INFO(ext);
    }

    assets.glPreferredTextureFormatRGBA = info.preferredFormatRGBA8;
    assets.glPreferredTextureFormatRGB = info.preferredFormatRGB8;

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

    ki::RenderClock& clock = m_clock;

    ki::FpsCounter fpsCounter;

    InputContext inputCtx{
        clock,
        m_registry.get(),
        m_window->m_input.get() };

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
            clock.ts = static_cast<float>(glfwGetTime());
            clock.elapsedSecs = elapsedDuration.count();

            if (m_registry->m_pendingSnapshotRegistry->isDirty()) {
                //KI_INFO("COPY: snapshot_dirty");
                m_registry->m_pendingSnapshotRegistry->copyTo(
                    m_registry->m_activeSnapshotRegistry,
                    0, -1);
            //}
            //else {
            //    KI_INFO("SKIP: snapshot_not_dirty");
            }

            // input
            // -----
            {
                m_window->processInput(inputCtx);
            }

            // render
            // ------
            {
                fpsCounter.startFame();

                // serious sync issue entity data vs. drawing
                // - looks like camera is jerky, but it's lack of sync between
                //   draw loop and update of UBOs & mapped buffers in next frame
                // => INEFFICIENT, need to improve this
                // https://forums.developer.nvidia.com/t/persistent-buffer-synchronization-doesnt-work/66636/5
                if (assets.glUseFinish) {
                    //std::cout << ".";
                    glFinish();
                }

                if (!close) {
                    close = onUpdate(clock);
                }
                if (!close) {
                    close = onRender(clock);
                }
                if (!close) {
                    close = onPost(clock);
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

    auto summary = fpsCounter.formatSummary(m_title.c_str());
    m_window->setTitle(summary);

    if (m_window->isFullScreen() && !assets.glVendorNvidia) {
        std::cout << summary << '\n';
    }
}

void Engine::onDestroy()
{
    *m_alive = false;
}

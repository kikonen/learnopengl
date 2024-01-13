#include "Engine.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <regex>

#include "imgui.h"

#include "util/Util.h"

#include "ki/Timer.h"

#include "kigl/kigl.h"
#include "kigl/OpenGLInfo.h"

#include "asset/MaterialSSBO.h"

#include "engine/AsyncLoader.h"
#include "engine/InputContext.h"

#include "registry/ProgramRegistry.h"
#include "registry/SnapshotRegistry.h"

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

    m_registry = std::make_shared<Registry>(m_assets, m_alive);
    m_asyncLoader = std::make_shared<AsyncLoader>(m_assets, m_alive);

    m_window = std::make_unique<Window>(*this);
    return m_window->create() ? 0 : -1;
}

int Engine::setup() {
    GLenum keys[] = {
        GL_BLEND,
        GL_CLIP_DISTANCE0,
        GL_CLIP_DISTANCE1,
        GL_CULL_FACE,
        GL_DEPTH_TEST,
        GL_POLYGON_OFFSET_FILL,
        GL_RASTERIZER_DISCARD,
        GL_STENCIL_TEST,
        GL_TEXTURE_CUBE_MAP_SEAMLESS,
    };
    for (auto& key : keys) {
        m_registry->m_state.track(key);
    }

    m_registry->prepareShared();

    return onSetup();
}

void Engine::run() {
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
    m_assets.glVendorNvidia = std::regex_match(vendor, std::regex(".*nvidia.*"));
    m_assets.glVendorIntel = std::regex_match(vendor, std::regex(".*intel.*"));

    KI_INFO("[EXTENSIONS]");
    for (const auto& ext : extensions) {
        KI_INFO(ext);
    }

    m_assets.glPreferredTextureFormatRGBA = info.preferredFormatRGBA8;
    m_assets.glPreferredTextureFormatRGB = info.preferredFormatRGB8;

    KI_INFO("setup");
    if (!m_assets.glNoError) {
        kigl::GL::startError();
    }

    if (m_assets.glDebug) {
        // NOTE KI MUST AFTER glfwWindow create
        kigl::GL::startDebug();
    }

    int res = setup();
    if (res) {
        m_window->close();
    }

    auto prevLoopTime = std::chrono::system_clock::now();
    auto loopTime = std::chrono::system_clock::now();

    auto renderStart = std::chrono::system_clock::now();
    auto renderEnd = std::chrono::system_clock::now();

    auto frameTime = std::chrono::system_clock::now();

    std::chrono::duration<float> elapsedDuration;
    std::chrono::duration<float> renderDuration;
    std::chrono::duration<float> frameDuration;

    ki::RenderClock& clock = m_clock;

    float frameSecs = 0;

    char titleSB[256];

    // NOTE KI moving avg of render time and fps
    constexpr int FPS_FRAMES = 10;
    int avgIndex = 0;
    std::array<float, FPS_FRAMES> fpsSecs{ 0.f };
    std::array<float, FPS_FRAMES> renderSecs{ 0.f };
    for (int i = 0; i < FPS_FRAMES; i++) {
        fpsSecs[i] = 0.f;
        renderSecs[i] = 0.f;
    }

    InputContext inputCtx{
        clock,
        m_assets,
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

            m_registry->m_snapshotRegistry->lock();

            // input
            // -----
            {
                m_window->processInput(inputCtx);
            }

            // render
            // ------
            {
                renderStart = std::chrono::system_clock::now();

                // serious sync issue entity data vs. drawing
                // - looks like camera is jerky, but it's lack of sync between
                //   draw loop and update of UBOs & mapped buffers in next frame
                // => INEFFICIENT, need to improve this
                // https://forums.developer.nvidia.com/t/persistent-buffer-synchronization-doesnt-work/66636/5
                if (m_assets.glUseFinish) {
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
                    m_registry->m_programRegistry->validate();
                }

                renderEnd = std::chrono::system_clock::now();
                renderDuration = renderEnd - renderStart;
                renderSecs[avgIndex] = renderDuration.count();
            }

            m_registry->m_snapshotRegistry->unlock();

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
            frameTime = std::chrono::system_clock::now();
            frameDuration = frameTime - loopTime;
            frameSecs = frameDuration.count();

            prevLoopTime = loopTime;

            //sprintf_s(
            //    titleSB,
            //    256,
            //    "%s - FPS: %-5.2f - RENDER: %-5.2fms FRAME: (%-5.2f fps)",
            //    m_title.c_str(),
            //    1.0f / clock.elapsedSecs,
            //    renderSecs * 1000.f,
            //    1.0f / frameSecs);

            fpsSecs[avgIndex] = clock.elapsedSecs;

            avgIndex = (avgIndex + 1) % FPS_FRAMES;

            // update title when wraparound
            if (avgIndex == 0)
            {
                float fpsTotal = 0.f;
                float renderTotal = 0.f;
                for (int i = 0; i < FPS_FRAMES; i++) {
                    fpsTotal += fpsSecs[i];
                    renderTotal += renderSecs[i];
                }

                float fpsAvg = (float)FPS_FRAMES / fpsTotal;
                float renderAvg = renderTotal * 1000.f / (float)FPS_FRAMES;

                sprintf_s(
                    titleSB,
                    256,
                    "%s - FPS: %-5.0f - RENDER: %-5.1fms",
                    m_title.c_str(),
                    round(fpsAvg),
                    renderAvg);

                m_window->setTitle(titleSB);

                if (m_window->isFullScreen() && !m_assets.glVendorNvidia) {
                    std::cout << titleSB << '\n';
                }
            }
        }

        //KI_GL_CHECK("engine.loop");
    }

    onDestroy();
}

void Engine::onDestroy()
{
    *m_alive = false;
}

#include "Engine.h"

#include <iostream>
#include <chrono>
#include <thread>

#include "imgui.h"

#include "ki/GL.h"
#include "ki/Timer.h"

#include "scene/AsyncLoader.h"


Engine::Engine()
{
}

Engine::~Engine() {
}

int Engine::init() {

    onInit();
    m_shaderRegistry = std::make_unique<ShaderRegistry>(m_assets);
    m_asyncLoader = std::make_shared<AsyncLoader>(m_assets);

    m_window = std::make_unique<Window>(*this, m_assets);
    return m_window->create() ? 0 : -1;
}

int Engine::setup() {
    GLenum keys[] = {
        GL_CULL_FACE,
        GL_FRONT_AND_BACK,
        GL_BLEND,
        GL_CLIP_DISTANCE0,
        GL_CLIP_DISTANCE1,
        GL_STENCIL_TEST,
        GL_DEPTH_TEST,
    };
    for (auto& key : keys) {
        m_state.track(key, false);
    }

    return onSetup();
}

void Engine::run() {
    const auto& info = ki::GL::getInfo();
    const auto& extensions = ki::GL::getExtensions();
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
GL_MAX_VERTEX_UNIFORM_COMPONENTS: {}
GL_MAX_VERTEX_ATTRIBS: {}
GL_PREFERRED_TEXTURE_FORMAT_RGBA8: 0x{:x}
GL_PREFERRED_TEXTURE_FORMAT_RGB8:  0x{:x}
)",
        info.vendor,
        info.renderer,
        info.version,
        info.glslVersion,
        info.maxVertexUniformComponents,
        info.maxVertexAttributes,
        info.preferredFormatRGBA8,
        info.preferredFormatRGB8));

    KI_INFO("[EXTENSIONS]");
    for (const auto& ext : extensions) {
        KI_INFO(ext);
    }

    m_assets.glPreferredTextureFormatRGBA = info.preferredFormatRGBA8;
    m_assets.glPreferredTextureFormatRGB = info.preferredFormatRGB8;

    KI_INFO("setup");
    if (!m_assets.glNoError) {
        ki::GL::startError();
    }

    if (m_assets.glDebug) {
        // NOTE KI MUST AFTER glfwWindow create
        ki::GL::startDebug();
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

    ki::RenderClock clock;

    float renderSecs = 0;
    float frameSecs = 0;

    char titleSB[256];

    float sleepSecs = 0;

    // render loop
    // -----------
    while (!m_window->isClosed())
    {
        int close = 0;

        {
            // make clear color by default black
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        }

        {
            //ki::Timer t("loop");


            loopTime = std::chrono::system_clock::now();
            elapsedDuration = loopTime - prevLoopTime;

            clock.frameCount += 1;
            clock.ts = glfwGetTime();
            clock.elapsedSecs = elapsedDuration.count();

            // input
            // -----
            m_window->processInput(clock);

            // render
            // ------
            {
                renderStart = std::chrono::system_clock::now();

                close = onRender(clock);

                m_shaderRegistry->validate();

                renderEnd = std::chrono::system_clock::now();
                renderDuration = renderEnd - renderStart;
                renderSecs = renderDuration.count();
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
            //glFinish();
        }

        if (!close) {
            frameTime = std::chrono::system_clock::now();
            frameDuration = frameTime - loopTime;
            frameSecs = frameDuration.count();

            prevLoopTime = loopTime;

            sprintf_s(
                titleSB,
                256,
                "%s - FPS: %-5.2f - RENDER: %-5.2fms FRAME: (%-5.2f fps)",
                m_title.c_str(),
                1.0f / clock.elapsedSecs,
                renderSecs * 1000.f,
                1.0f / frameSecs);

            m_window->setTitle(titleSB);
        }

        KI_GL_CHECK("engine.loop");
    }

    onDestroy();
}

void Engine::onDestroy()
{
}

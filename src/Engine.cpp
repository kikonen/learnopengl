#include "Engine.h"

#include <iostream>
#include <chrono>
#include <thread>

#include "imgui.h"

#include "ki/GL.h"
#include "ki/Timer.h"

#include "scene/AsyncLoader.h"

//std::shared_ptr<Engine> Engine::current = nullptr;

Engine::Engine()
{
    debug = false;
    throttleFps = FPS_15;
    window = std::make_unique<Window>(*this);
    asyncLoader = std::make_shared<AsyncLoader>(shaders, assets);
}

Engine::~Engine() {
}

int Engine::init() {
    return window->create() ? 0 : -1;
}

void Engine::run() {
    OpenGLInfo info = ki::GL::getInfo();
    KI_INFO_SB("ENGINE::RUN" << std::endl
        << " VER=" << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
        << " GL_MAX_VERTEX_UNIFORM_COMPONENTS=" << info.maxVertexUniformComponents << std::endl
        << " GL_MAX_VERTEX_ATTRIBS=" << info.maxVertexAttributes);

    KI_INFO("setup");
    ki::GL::startError();
    ki::GL::startDebug();

    int res = onSetup();
    if (res) {
        window->close();
    }

    auto prevLoopTime = std::chrono::system_clock::now();
    auto loopTime = std::chrono::system_clock::now();

    auto renderStart = std::chrono::system_clock::now();
    auto renderEnd = std::chrono::system_clock::now();

    auto frameTime = std::chrono::system_clock::now();

    std::chrono::duration<float> elapsedDuration;
    std::chrono::duration<float> renderDuration;
    std::chrono::duration<float> frameDuration;

    RenderClock clock;

    float renderSecs = 0;
    float frameSecs = 0;

    char titleSB[256];

    float sleepSecs = 0;

    // render loop
    // -----------
    while (!window->isClosed())
    {
        int close = 0;
        {
            //ki::Timer t("loop");

            loopTime = std::chrono::system_clock::now();
            elapsedDuration = loopTime - prevLoopTime;

            clock.ts = glfwGetTime();
            clock.elapsedSecs = elapsedDuration.count();

            // input
            // -----
            window->processInput(clock);

            // render
            // ------
            {
                renderStart = std::chrono::system_clock::now();

                close = onRender(clock);

                renderEnd = std::chrono::system_clock::now();
                renderDuration = renderEnd - renderStart;
                renderSecs = renderDuration.count();
            }

            if (close) {
                window->close();
            }
        }


        if (!close) {
            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window->glfwWindow);
            glfwPollEvents();
            //glFinish();
        }

        if (!close) {
            frameTime = std::chrono::system_clock::now();
            frameDuration = frameTime - loopTime;
            frameSecs = frameDuration.count();

            prevLoopTime = loopTime;

            sprintf_s(titleSB, 256, "%s - FPS: %3.2f - RENDER: %3.2fms FRAME: (%3.2f fps)", title.c_str(), 1.0f / clock.elapsedSecs, renderSecs * 1000.f, 1.0f / frameSecs);
            window->setTitle(titleSB);
            //KI_DEBUG_SB(titleSB);
        }

        KI_GL_CHECK("engine.loop");

        // NOTE KI aim 60fps (no reason to overheat CPU/GPU)
        if (!close && throttleFps > 0) {
            sleepSecs = throttleFps / 1000.f - renderSecs * 2;
            if (sleepSecs < 0) {
                sleepSecs = 0.01;
            }
            //KI_DEBUG_SB("dt: " << elapsedSecs * 1000.f << "ms - " << "render: " << renderSecs * 1000 << "ms - " << "sleep: " << sleepSecs * 1000 << "ms");
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(sleepSecs * 1000.f)));
        }
    }

    onDestroy();
}

void Engine::onDestroy()
{
}

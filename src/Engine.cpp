#include "Engine.h"

#include <iostream>
#include <chrono>
#include <thread>

#include "imgui.h"

#include "ki/GL.h"
#include "ki/Timer.h"

Engine* Engine::current = nullptr;

Engine::Engine() {
	debug = false;
	throttleFps = FPS_15;
	window = new Window(*this);
}

Engine::~Engine() {
	delete window;
}

int Engine::init() {
	return window->create() ? 0 : -1;
}

void Engine::run() {
	OpenGLInfo info = ki::GL::getInfo();
	KI_INFO_SB("ENGINE::INIT" << std::endl
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
    auto renderTime = std::chrono::system_clock::now();

	std::chrono::duration<float> elapsedDuration;
	std::chrono::duration<float> renderDuration;

	RenderClock clock;

	float renderSecs = 0;

	char titleSB[256];

	float sleepSecs = 0;

	// render loop
	// -----------
	while (!window->isClosed())
	{
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
			int res = onRender(clock);

			if (res) {
				window->close();
			}

			// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
			// -------------------------------------------------------------------------------
			glfwSwapBuffers(window->glfwWindow);
			glfwPollEvents();
			//glFinish();
		}

		{
			renderTime = std::chrono::system_clock::now();
			renderDuration = renderTime - loopTime;
			renderSecs = renderDuration.count();

			prevLoopTime = loopTime;

			sprintf_s(titleSB, 256, "%s - FPS: %3.2f - RENDER: %3.2fms (%3.2f fps)", title.c_str(), 1.0f / clock.elapsedSecs, renderSecs * 1000.f, 1.0f / renderSecs);
			window->setTitle(titleSB);
			//KI_DEBUG_SB(titleSB);
		}

		KI_GL_CHECK("engine.loop");

		// NOTE KI aim 60fps (no reason to overheat CPU/GPU)
		if (throttleFps > 0) {
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


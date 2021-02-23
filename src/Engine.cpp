#include "Engine.h"

#include <iostream>
#include <fstream>
#include <strstream>
#include <chrono>
#include <thread>
#include <functional>

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
	// uncomment this call to draw in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	OpenGLInfo info = ki::GL::getInfo();
	std::cout << "ENGINE::INIT" << std::endl
		<< " VER=" << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
		<< " GL_MAX_VERTEX_UNIFORM_COMPONENTS=" << info.maxVertexUniformComponents << std::endl
		<< " GL_MAX_VERTEX_ATTRIBS=" << info.maxVertexAttributes << std::endl;

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
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			int res = onRender(clock);

			if (res) {
				window->close();
			}

			// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
			// -------------------------------------------------------------------------------
			glfwSwapBuffers(window->glfwWindow);
			glfwPollEvents();
		}

		{
			renderTime = std::chrono::system_clock::now();
			renderDuration = renderTime - loopTime;
			renderSecs = renderDuration.count();

			prevLoopTime = loopTime;

			sprintf_s(titleSB, 256, "%s - FPS: %3.2f - RENDER: %3.2fms (%3.2f fps)", title.c_str(), 1.0f / clock.elapsedSecs, renderSecs * 1000.f, 1.0f / renderSecs);
			window->setTitle(titleSB);
			//std::cout << titleSB << "\n";
		}

		KI_GL_CHECK("engine.loop");

		// NOTE KI aim 60fps (no reason to overheat CPU/GPU)
		if (throttleFps > 0) {
			sleepSecs = throttleFps / 1000.f - renderSecs * 2;
			if (sleepSecs < 0) {
				sleepSecs = 0.01;
			}
			//std::cout << "dt: " << elapsedSecs * 1000.f << "ms - " << "render: " << renderSecs * 1000 << "ms - " << "sleep: " << sleepSecs * 1000 << "ms\n";
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(sleepSecs * 1000.f)));
		}
	}

	onDestroy();
}

void Engine::onDestroy()
{
}


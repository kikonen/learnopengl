#include "Engine.h"

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

	int maxUniforms;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxUniforms);
	std::cout << "ENGINE::INIT" 
		<< " VER=" << glGetString(GL_SHADING_LANGUAGE_VERSION) 
		<< " MAX_UNIFORMS=" << maxUniforms
		<< std::endl;
	int res = onSetup();
	if (res) {
		window->close();
	}

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();
    auto tp3 = std::chrono::system_clock::now();

	std::chrono::duration<float> duration;
	float ts;
	char titleSB[256];

	std::chrono::duration<float> elapsedTime;

	// render loop
	// -----------
	while (!window->isClosed())
	{
		float dt;
		{
			//ki::Timer t("loop");

			tp2 = std::chrono::system_clock::now();
			elapsedTime = tp2 - tp1;
			dt = elapsedTime.count();

			accumulatedTime += dt;

			// input
			// -----
			window->processInput(dt);

			// render
			// ------
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			int res = onRender(dt);

			if (res) {
				window->close();
			}

			// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
			// -------------------------------------------------------------------------------
			glfwSwapBuffers(window->glfwWindow);
			glfwPollEvents();
		}

		{
			tp3 = std::chrono::system_clock::now();
			duration = tp3 - tp2;
			ts = duration.count() * 1000;

			tp1 = tp2;
			sprintf_s(titleSB, 256, "%s - FPS: %3.2f - %3.2fms", title.c_str(), 1.0f / dt, ts);
			window->setTitle(titleSB);
		}

		ki::GL::checkErrors("engine.loop");

		// NOTE KI aim 60fps (no reason to overheat CPU/GPU)
		if (throttleFps > 0) {
			int sleep = throttleFps - ts;
			if (sleep < 0) {
				sleep = 10;
			}
//			std::cout << "sleep: " << sleep << "ms\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
		}
	}

	onDestroy();
}

void Engine::onDestroy()
{
}


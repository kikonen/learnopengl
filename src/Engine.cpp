#include "Engine.h"


Engine::Engine() {
	title = "GL test";
	width = 800;
	height = 600;
	debug = false;
	throttleFps = FPS_15;
}

Engine::~Engine() {
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
}

int Engine::init() {
	window = createWindow();
	if (!window) {
		return -1;
	}
	return 0;
}

void Engine::run() {
	// uncomment this call to draw in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	int res = onSetup();
	if (res) {
		glfwSetWindowShouldClose(window, true);
	}

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();
    auto tp3 = std::chrono::system_clock::now();

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		float dt = elapsedTime.count();

		accumulatedTime += dt;

		// input
		// -----
		processInput(window, dt);

		// render
		// ------
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		int res = onRender(dt);

		if (res) {
			glfwSetWindowShouldClose(window, true);
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();

		tp3 = std::chrono::system_clock::now();
        std::chrono::duration<float> loopTime = tp3 - tp2;
        float ts = loopTime.count() * 1000;

		char s[256];
		sprintf_s(s, 256, "%s - FPS: %3.2f - %3.2fms", title.c_str(), 1.0f / dt, ts);
		glfwSetWindowTitle(window, s);

		tp1 = tp2;

		// NOTE KI aim 60fps (no reason to overheat CPU/GPU)
		if (throttleFps > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(throttleFps));
		}
	}
}

GLFWwindow* Engine::createWindow() {
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return NULL;
	}

	return window;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Engine::processInput(GLFWwindow* window, float dt)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	camera.processInput(window, dt);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void Engine::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

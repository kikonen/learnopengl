#include "Engine.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


Engine* Engine::current = nullptr;

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

	int maxUniforms;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxUniforms);
	std::cout << "ENGINE::INIT" 
		<< " VER=" << glGetString(GL_SHADING_LANGUAGE_VERSION) 
		<< " MAX_UNIFORMS=" << maxUniforms
		<< std::endl;
	int res = onSetup();
	if (res) {
		glfwSetWindowShouldClose(window, true);
	}

	onSetupGUI();

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
		processInput(dt);

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

	onDestroyGUI();
	onDestroy();
}

void Engine::onDestroy()
{
}

void Engine::onSetupGUI()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	// Setup Dear ImGui style
	ImGui::StyleColorsLight();
}

void Engine::onDestroyGUI()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

Shader* Engine::getShader(const std::string& name, const std::string& geometryType)
{
	return Shader::getShader(assets, name, geometryType);
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

	// callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return NULL;
	}

	input = new Input(window);

	return window;
}

void Engine::processInput(float dt)
{
	if (input->isPressed(Key::EXIT)) {
		glfwSetWindowShouldClose(window, true);
	}
	camera.onKey(input, dt);
}

void Engine::on_framebuffer_size(int width, int height)
{
	glViewport(0, 0, width, height);
	this->width = width;
	this->height = height;
}

void Engine::on_mouse(double xpos, double ypos)
{
	input->handleMouse(xpos, ypos);

	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		camera.onMouseMove(input, input->mouseXoffset, input->mouseYoffset);
	}
	else {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void Engine::on_scroll(double xoffset, double yoffset)
{
	camera.onMouseScroll(input, xoffset, yoffset);
}

void Engine::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	current->on_framebuffer_size(width, height);
}

void Engine::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	current->on_mouse(xpos, ypos);
}

void Engine::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	current->on_scroll(xoffset, yoffset);
}

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>


/**
* https://learnopengl.com/Getting-started/Camera
*/
class Camera
{
public:
	Camera();
	~Camera();

	void handleInput(GLFWwindow* window, float dt);

	glm::mat4 updateCamera(float dt);
private:

	glm::vec3 cameraPos;
	glm::vec3 cameraTarget;
	glm::vec3 cameraDirection;

	glm::vec3 up;
	glm::vec3 cameraRight;
	glm::vec3 cameraUp;

	float accumulatedTime = 0;
};


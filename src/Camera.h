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

	const glm::mat4& getView();

	void onKey(GLFWwindow* window, float dt);
	void onMouseMove(GLFWwindow* window, float xoffset, float yoffset);
	void onMouseScroll(GLFWwindow* window, float xoffset, float yoffset);

private:
	float moveStep = 8.0f;
	float rotateStep = 20.f;
	float mouseSensitivity = 0.1f;
	float zoom = 1.0f;

	glm::vec3 pos;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;

	glm::mat4 viewMat;
	glm::mat4 rotateMat;

	glm::vec3 viewFront;
	glm::vec3 viewRight;
	glm::vec3 viewUp;

	float accumulatedTime = 0;

	bool dirty;

	void updateCamera();
	void updateRotate(glm::mat4& rot, float angleX, float angleY, float angleZ);
};


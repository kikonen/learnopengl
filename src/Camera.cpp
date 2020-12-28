#include "Camera.h"

Camera::Camera()
{
	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);

	updateCamera(0);
}

Camera::~Camera()
{
}

void Camera::handleInput(GLFWwindow* window, float dt)
{
}

glm::mat4 Camera::updateCamera(float dt)
{
	accumulatedTime += dt;

	cameraDirection = glm::normalize(cameraPos - cameraTarget);
	cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	cameraUp = glm::cross(cameraDirection, cameraRight);

/*
glm::mat4 view;
	view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 3.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
*/
	const float radius = 10.0f;
	float camX = sin(accumulatedTime) * radius;
    float camY = sin(accumulatedTime) * radius / 4;
    float camZ = cos(accumulatedTime) * radius / 2;

	glm::mat4 view;
	view = glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	return view;
}


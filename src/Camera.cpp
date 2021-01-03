#include "Camera.h"

Camera::Camera()
{
	pos = glm::vec3(0.0f, 0.0f, 5.0f);

	front = glm::vec3(0.0f, 0.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	right = glm::cross(front, up);

	rotateMat = createRotate(0.f, 0.f, 0.f);

	dirty = true;
	updateCamera();
}

Camera::~Camera()
{
}

const glm::mat4& Camera::getView()
{
	if (!dirty) {
		return viewMat;
	}

	updateCamera();
	viewMat = glm::lookAt(
		pos,
		pos + viewFront,
		viewUp);
	return viewMat;
}

void Camera::processInput(GLFWwindow* window, float dt)
{
	accumulatedTime += dt;

	const float moveSpeed = 7.0f;
	const float rotateSpeed = 0.9f;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		updateCamera();
		this->pos += viewFront * dt * moveSpeed;
		dirty = true;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		updateCamera();
		this->pos -= viewFront * dt * moveSpeed;
		dirty = true;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		updateCamera();
		this->pos -= viewRight * dt * moveSpeed;
		dirty = true;
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		updateCamera();
		this->pos += viewRight * dt * moveSpeed;
		dirty = true;
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		updateCamera();
		this->pos += viewUp * dt * moveSpeed;
		dirty = true;
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		updateCamera();
		this->pos -= viewUp * dt * moveSpeed;
		dirty = true;
	}

	float angleY = 0.0f;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		angleY += rotateSpeed * dt;
		dirty = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		angleY -= rotateSpeed * dt;
		dirty = true;
	}

	if (angleY) {
		rotateMat = glm::rotate(
			rotateMat,
			angleY,
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
		dirty = true;
	}
}

void Camera::updateCamera()
{
	if (!dirty) {
		return;
	}

	viewFront = glm::normalize(rotateMat * glm::vec4(front, 1.f));
	viewUp = glm::normalize(rotateMat * glm::vec4(up, 1.f));
	viewRight = glm::cross(viewFront, viewUp);
}

glm::mat4 Camera::createRotate(float angleX, float angleY, float angleZ)
{
	// ORDER: yaw - pitch - roll
	glm::mat4 rot = glm::mat4(1.0f);
	rot = glm::rotate(
		rot,
		angleX,
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	rot = glm::rotate(
		rot,
		angleY,
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	rot = glm::rotate(
		rot,
		angleZ,
		glm::vec3(-1.0f, 0.0f, 0.0f)
	);
	return rot;
}

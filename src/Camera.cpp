#include "Camera.h"

Camera::Camera()
{
	pos = glm::vec3(0.0f, 0.0f, 5.0f);

	front = glm::vec3(0.0f, 0.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	right = glm::cross(front, up);

	rotateMat = glm::mat4(1.0f);
	updateRotate(rotateMat, 0.f, 0.f, 0.f);

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

void Camera::onKey(GLFWwindow* window, float dt)
{
	accumulatedTime += dt;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		updateCamera();
		this->pos += viewFront * dt * moveStep;
		dirty = true;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		updateCamera();
		this->pos -= viewFront * dt * moveStep;
		dirty = true;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		updateCamera();
		this->pos -= viewRight * dt * moveStep;
		dirty = true;
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		updateCamera();
		this->pos += viewRight * dt * moveStep;
		dirty = true;
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		updateCamera();
		this->pos += viewUp * dt * moveStep;
		dirty = true;
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		updateCamera();
		this->pos -= viewUp * dt * moveStep;
		dirty = true;
	}

	float angleX = 0.0f;
	float angleY = 0.0f;
	float angleZ = 0.0f;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		angleY += rotateStep * dt;
		dirty = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		angleY -= rotateStep * dt;
		dirty = true;
	}

	if (angleX || angleY || angleZ) {
		updateRotate(rotateMat, angleX, angleY, angleZ);
		dirty = true;
	}
}

void Camera::onMouseMove(GLFWwindow* window, float xoffset, float yoffset)
{
	float angleX = 0.0f;
	float angleY = 0.0f;
	float angleZ = 0.0f;

	const float MAX_CHANGE = 25.f;

	if (true) {
		angleY -= mouseSensitivity * xoffset;

		if (angleY < -MAX_CHANGE) {
			angleY = -MAX_CHANGE;
		}
		if (angleY > MAX_CHANGE) {
			angleY = MAX_CHANGE;
		}
	}

	if (true) {
		angleX += mouseSensitivity * yoffset;

		if (angleX < -MAX_CHANGE) {
			angleX = -MAX_CHANGE;
		}
		if (angleX > MAX_CHANGE) {
			angleX = MAX_CHANGE;
		}
	}

	if (angleX || angleY || angleZ) {
		updateRotate(rotateMat, angleX, angleY, angleZ);
		dirty = true;
	}
}

void Camera::onMouseScroll(GLFWwindow* window, float xoffset, float yoffset)
{
	zoom -= yoffset;
	if (zoom <= 0.1f) {
		zoom = 0.1f;
	}
	if (zoom <= 10.f) {
		zoom = 10.f;
	}
	dirty = true;
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

void Camera::updateRotate(glm::mat4& rot, float angleX, float angleY, float angleZ)
{
	// ORDER: yaw - pitch - roll
	rot = glm::rotate(
		rot,
		glm::radians(angleX),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	rot = glm::rotate(
		rot,
		glm::radians(angleY),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	rot = glm::rotate(
		rot,
		glm::radians(angleZ),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
}

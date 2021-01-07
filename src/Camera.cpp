#include "Camera.h"

const float MIN_ZOOM = 1.0f;
const float MAX_ZOOM = 45.0f;


Camera::Camera()
{
	pos = glm::vec3(0.0f, 0.0f, 5.0f);

	front = glm::vec3(0.0f, 0.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	right = glm::normalize(glm::cross(front, up));

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

void Camera::setPos(const glm::vec3& pos) {
	this->pos = pos;
	dirty = true;
}

const glm::vec3& Camera::getPos() const {
	return pos;
}

void Camera::onKey(Input* input, float dt)
{
	accumulatedTime += dt;

	if (input->isPressed(Key::FORWARD)) {
		updateCamera();
		this->pos += viewFront * dt * moveStep;
		dirty = true;
	}

	if (input->isPressed(Key::BACKWARD)) {
		updateCamera();
		this->pos -= viewFront * dt * moveStep;
		dirty = true;
	}

	if (input->isPressed(Key::LEFT)) {
		updateCamera();
		this->pos -= viewRight * dt * moveStep;
		dirty = true;
	}

	if (input->isPressed(Key::RIGHT)) {
		updateCamera();
		this->pos += viewRight * dt * moveStep;
		dirty = true;
	}

	if (input->isPressed(Key::UP)) {
		updateCamera();
		this->pos += viewUp * dt * moveStep;
		dirty = true;
	}

	if (input->isPressed(Key::DOWN)) {
		updateCamera();
		this->pos -= viewUp * dt * moveStep;
		dirty = true;
	}

	if (true) {
		if (input->isPressed(Key::ROTATE_LEFT)) {
			yaw += rotateStep * dt;
			dirty = true;
		}
		if (input->isPressed(Key::ROTATE_RIGHT)) {
			yaw -= rotateStep * dt;
			dirty = true;
		}

		updateRotate(rotateMat, yaw, pitch, roll);
	}

	if (input->isPressed(Key::ZOOM_IN)) {
		updateZoom(zoom - zoomStep * dt);
	}
	if (input->isPressed(Key::ZOOM_OUT)) {
		updateZoom(zoom + zoomStep * dt);
	}
}

void Camera::onMouseMove(Input* input, float xoffset, float yoffset)
{
	const float MAX_ANGLE = 89.f;

	if (true) {
		yaw -= mouseSensitivity * xoffset;
	}

	if (true) {
		pitch += mouseSensitivity * yoffset;

		if (pitch < -MAX_ANGLE) {
			pitch = -MAX_ANGLE;
		}
		if (pitch > MAX_ANGLE) {
			pitch = MAX_ANGLE;
		}
	}

	updateRotate(rotateMat, yaw, pitch, roll);
	dirty = true;
}

void Camera::onMouseScroll(Input* input, float xoffset, float yoffset)
{
	updateZoom(zoom - yoffset);
}

void Camera::updateZoom(float aZoom)
{
	if (aZoom < MIN_ZOOM) {
		aZoom = MIN_ZOOM;
	}
	if (aZoom > MAX_ZOOM) {
		aZoom = MAX_ZOOM;
	}
	if (aZoom != zoom) {
		zoom = aZoom;
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
	viewRight = glm::normalize(glm::cross(viewFront, viewUp));
}

void Camera::updateRotate(glm::mat4& rot, float yaw, float pitch, float roll)
{
	rot = glm::mat4(1.f);

	// ORDER: yaw - pitch - roll
	rot = glm::rotate(
		rot,
		glm::radians(yaw),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	rot = glm::rotate(
		rot,
		glm::radians(pitch),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	rot = glm::rotate(
		rot,
		glm::radians(roll),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
}

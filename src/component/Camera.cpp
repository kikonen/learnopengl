#include "Camera.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


const float MIN_ZOOM = 1.0f;
const float MAX_ZOOM = 45.0f;


Camera::Camera(const glm::vec3& aPos, const glm::vec3 aFront, const glm::vec3 aUp)
{
	pos = aPos;

	// Default: look to Z direction
	front = glm::normalize(aFront);
	up = glm::normalize(aUp);
	right = glm::normalize(glm::cross(front, up));

	rotateMat = glm::mat4(1.0f);

	dirty = true;
}

Camera::~Camera()
{
}

const glm::mat4& Camera::getView()
{
	if (!dirty) return viewMat;

	updateCamera();
	viewMat = glm::lookAt(
		pos,
		pos + viewFront,
		viewUp);
	return viewMat;
}

const glm::vec3& Camera::getViewFront()
{
	if (dirty) updateCamera();
	return viewFront;
}

const glm::vec3& Camera::getViewRight()
{
	if (dirty) updateCamera();
	return viewRight;
}

const glm::vec3& Camera::getViewUp()
{
	if (dirty) updateCamera();
	return viewUp;
}


const glm::vec3& Camera::getFront()
{
	return front;
}

const glm::vec3& Camera::getRight()
{
	return right;
}

const glm::vec3& Camera::getUp()
{
	return up;
}

float Camera::getZoom()
{
	return zoom;
}

void Camera::setZoom(float zoom)
{
	this->zoom = zoom;
	dirty = true;
}

void Camera::setPos(const glm::vec3& pos) {
	this->pos = pos;
	dirty = true;
}

const glm::vec3& Camera::getPos() const {
	return pos;
}

void Camera::setRotation(const glm::vec3& rotation)
{
	yaw = rotation.y;
	pitch = rotation.x;
	roll = rotation.z;
}

const glm::vec3 Camera::getRotation()
{
	return glm::vec3(pitch, yaw, roll);
}

void Camera::onKey(Input* input, float dt)
{
	float moveSize = moveStep;
	float rotateSize = rotateStep;
	if (input->isModifier(Modifier::SHIFT)) {
		moveSize *= 2;
		rotateSize *= 2;
	}

	if (input->isKeyPressed(Key::FORWARD)) {
		updateCamera();
		this->pos += viewFront * dt * moveSize;
		dirty = true;
	}

	if (input->isKeyPressed(Key::BACKWARD)) {
		updateCamera();
		this->pos -= viewFront * dt * moveSize;
		dirty = true;
	}

	if (input->isKeyPressed(Key::LEFT)) {
		updateCamera();
		this->pos -= viewRight * dt * moveSize;
		dirty = true;
	}

	if (input->isKeyPressed(Key::RIGHT)) {
		updateCamera();
		this->pos += viewRight * dt * moveSize;
		dirty = true;
	}

	if (input->isKeyPressed(Key::UP)) {
		updateCamera();
		this->pos += viewUp * dt * moveSize;
		dirty = true;
	}

	if (input->isKeyPressed(Key::DOWN)) {
		updateCamera();
		this->pos -= viewUp * dt * moveSize;
		dirty = true;
	}

	if (true) {
		if (input->isKeyPressed(Key::ROTATE_LEFT)) {
			yaw += rotateSize * dt;
			dirty = true;
		}
		if (input->isKeyPressed(Key::ROTATE_RIGHT)) {
			yaw -= rotateSize * dt;
			dirty = true;
		}
	}

	if (input->isKeyPressed(Key::ZOOM_IN)) {
		updateZoom(zoom - zoomStep * dt);
	}
	if (input->isKeyPressed(Key::ZOOM_OUT)) {
		updateZoom(zoom + zoomStep * dt);
	}
}

void Camera::onMouseMove(Input* input, double xoffset, double yoffset)
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

	dirty = true;
}

void Camera::onMouseScroll(Input* input, double xoffset, double yoffset)
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

	// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
	rotateMat = glm::toMat4(glm::quat(glm::radians(glm::vec3(pitch, yaw, roll))));

	// NOTE KI glm::normalize for vec4 *IS* incorrect (4d len...)
	viewFront = glm::normalize(glm::vec3(rotateMat * glm::vec4(front, 1.f)));
	viewUp = glm::normalize(glm::vec3(rotateMat * glm::vec4(up, 1.f)));
	viewRight = glm::normalize(glm::cross(viewFront, viewUp));
}

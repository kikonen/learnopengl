#include "Camera.h"

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

const glm::vec3& Camera::getFront()
{
	return front;
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
	if (input->isKeyPressed(Key::FORWARD)) {
		updateCamera();
		this->pos += viewFront * dt * moveStep;
		dirty = true;
	}

	if (input->isKeyPressed(Key::BACKWARD)) {
		updateCamera();
		this->pos -= viewFront * dt * moveStep;
		dirty = true;
	}

	if (input->isKeyPressed(Key::LEFT)) {
		updateCamera();
		this->pos -= viewRight * dt * moveStep;
		dirty = true;
	}

	if (input->isKeyPressed(Key::RIGHT)) {
		updateCamera();
		this->pos += viewRight * dt * moveStep;
		dirty = true;
	}

	if (input->isKeyPressed(Key::UP)) {
		updateCamera();
		this->pos += viewUp * dt * moveStep;
		dirty = true;
	}

	if (input->isKeyPressed(Key::DOWN)) {
		updateCamera();
		this->pos -= viewUp * dt * moveStep;
		dirty = true;
	}

	if (true) {
		if (input->isKeyPressed(Key::ROTATE_LEFT)) {
			yaw += rotateStep * dt;
			dirty = true;
		}
		if (input->isKeyPressed(Key::ROTATE_RIGHT)) {
			yaw -= rotateStep * dt;
			dirty = true;
		}

		updateRotate(rotateMat, yaw, pitch, roll);
	}

	if (input->isKeyPressed(Key::ZOOM_IN)) {
		updateZoom(zoom - zoomStep * dt);
	}
	if (input->isKeyPressed(Key::ZOOM_OUT)) {
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

	updateRotate(rotateMat, yaw, pitch, roll);

	// NOTE KI glm::normalize for vec4 *IS* incorrect (4d len...)
	viewFront = glm::normalize(glm::vec3(rotateMat * glm::vec4(front, 1.f)));
	viewUp = glm::normalize(glm::vec3(rotateMat * glm::vec4(up, 1.f)));
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

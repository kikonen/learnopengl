#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Input.h"

/**
* https://learnopengl.com/Getting-started/Camera
*/
class Camera final
{
public:
	Camera();
	~Camera();

	const glm::mat4& getView();

	const glm::vec3& getFront();

	void setPos(const glm::vec3& pos);
	const glm::vec3& getPos() const;

	void setRotation(const glm::vec3& rotation);
	const glm::vec3 getRotation();

	void onKey(Input* input, float dt);
	void onMouseMove(Input* input, float xoffset, float yoffset);
	void onMouseScroll(Input* input, float xoffset, float yoffset);

public:
	float zoom = 45.0f;

private:
	float moveStep = 10.0f;
	float rotateStep = 30.f;
	float zoomStep = 20.0f;
	float mouseSensitivity = 0.1f;

	glm::vec3 pos;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;

	glm::mat4 viewMat;
	glm::mat4 rotateMat;

	glm::vec3 viewFront;
	glm::vec3 viewRight;
	glm::vec3 viewUp;

	float yaw = 0;
	float pitch = 0;
	float roll = 0;

	float accumulatedTime = 0;

	bool dirty;

	void updateZoom(float aZoom);

	void updateCamera();
	void updateRotate(glm::mat4& rot, float angleX, float angleY, float angleZ);
};


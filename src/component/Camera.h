#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "gui/Input.h"

/**
* https://learnopengl.com/Getting-started/Camera
*/
class Camera final
{
public:
    Camera(const glm::vec3& pos, const glm::vec3 front, const glm::vec3 aUp);
    ~Camera();

    const glm::mat4& getView();
    const glm::vec3& getViewFront();
    const glm::vec3& getViewRight();
    const glm::vec3& getViewUp();

    const glm::vec3& getFront();
    const glm::vec3& getRight();
    const glm::vec3& getUp();

    double getZoom();
    void setZoom(double zoom);

    void setPos(const glm::vec3& pos);
    const glm::vec3& getPos() const;

    void setRotation(const glm::vec3& rotation);
    const glm::vec3 getRotation();

    void onKey(Input* input, const RenderClock& clock);
    void onMouseMove(Input* input, double xoffset, double yoffset);
    void onMouseScroll(Input* input, double xoffset, double yoffset);

private:
    void updateZoom(double aZoom);
    void updateCamera();

private:
    double zoom = 45.0f;

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

    bool dirty = true;
};


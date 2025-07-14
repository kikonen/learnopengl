#pragma once

#include <glm/glm.hpp>

#include "kigl/kigl.h"

// NOTE KI align 16 for UBO struct
#pragma pack(push, 1)
struct CameraUBO {
    glm::vec3 u_cameraPos;
    //int pad_u_cameraPos;

    glm::vec3 u_cameraFront;
    //int pad_u_cameraFront;

    glm::vec3 u_cameraUp;
    //int pad_u_cameraUp;

    glm::vec3 u_cameraRight;
    //int pad_u_cameraRight;

    glm::vec3 u_mainCameraPos;
    //int pad_u_mainCameraPos;

    glm::vec3 u_mainCameraFront;
    //int pad_u_mainCameraFront;

    glm::vec3 u_mainCameraUp;
    //int pad_u_mainCameraUp;

    glm::vec3 u_mainCameraRight;
    //int pad_u_mainCameraRight;

    float u_nearPlane;
    float u_farPlane;

    int pad1;
    int pad2;
};
#pragma pack(pop)

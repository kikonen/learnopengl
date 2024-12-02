#pragma once

#include <glm/glm.hpp>

// NOTE KI align 16 for UBO struct
#pragma pack(push, 1)
struct DataUBO {
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

    glm::vec4 u_fogColor;

    float u_nearPlane;
    float u_farPlane;

    int u_cubeMapExist; // bool as 4 bytes
    int u_skyboxExist; // bool as 4 bytes

    int u_environmentMapExist; // bool as 4 bytes

    int u_shadowVisual; // bool as 4 bytes

    float u_fogStart;
    float u_fogEnd;
    float u_fogDensity;

    float u_hdrGamma;
    float u_hdrExposure;

    float u_time;
    int u_frame;
    int u_shadowCount;

    // From *camera* view (not shadow view)
    // NOTE KI std410 arrays are glm::vec4 *alignment* per item
    float u_shadowCascade_0;
    float u_shadowCascade_1;
    float u_shadowCascade_2;
    float u_shadowCascade_3;

    int pad1;
    int pad2;
};
#pragma pack(pop)

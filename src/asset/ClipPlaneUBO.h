#pragma once

#include <glm/glm.hpp>

constexpr unsigned int MIN_CLIP_PLANE_COUNT = 2;
constexpr unsigned int MAX_CLIP_PLANE_COUNT = 2;
constexpr unsigned int CLIP_PLANE_COUNT = MAX_CLIP_PLANE_COUNT;


// NOTE KI align 16 for UBO struct
struct ClipPlaneUBO {
    glm::vec4 plane;
    bool enabled;

    int pad1;
    int pad2;
    int pad3;
};

// NOTE KI align 16 for UBO struct
struct ClipPlanesUBO {
    // NOTE KI align 16 for UBO array entries
    ClipPlaneUBO clipping[CLIP_PLANE_COUNT];
    int clipCount;

    int pad1;
    int pad2;
    int pad3;
};

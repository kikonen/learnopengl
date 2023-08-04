#pragma once

#include <glm/glm.hpp>

constexpr unsigned int MIN_CLIP_PLANE_COUNT = 2;
constexpr unsigned int MAX_CLIP_PLANE_COUNT = 2;
constexpr unsigned int CLIP_PLANE_COUNT = MAX_CLIP_PLANE_COUNT;

#pragma pack(push, 1)

// NOTE KI align 16 for UBO struct
// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct ClipPlaneUBO {
    glm::vec4 u_plane;
};

// NOTE KI align 16 for UBO struct
struct ClipPlanesUBO {
    int u_clipCount;

    int pad1;
    int pad2;
    int pad3;

    // NOTE KI align 16 for UBO array entries
    ClipPlaneUBO u_clipping[CLIP_PLANE_COUNT];
};

#pragma pack(pop)

#pragma once

#include "assimp_util.h"

namespace assimp_util {
    glm::vec4 toVec4(const aiColor4D& v) {
        return { v.r, v.g, v.b, v.a };
    }

    glm::vec3 toVec3(const aiVector3D& v) {
        return { v.x, v.y, v.z };
    }

    glm::vec2 toVec2(const aiVector3D& v) {
        return { v.x, v.y };
    }

    glm::mat4 toMat4(const aiMatrix4x4& v) {
        return {
            v.a1, v.b1, v.c1, v.d1,
            v.a2, v.b2, v.c2, v.d2,
            v.a3, v.b3, v.c3, v.d3,
            v.a4, v.b4, v.c4, v.d4,
        };
    }

    glm::quat toQuat(const aiQuaternion& v) {
        return { v.w, v.x, v.y, v.z };
    }
}

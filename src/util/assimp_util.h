#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/color4.h>
#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>
#include <assimp/material.h>

#include "util/Transform.h"
#include "util/UVTransform.h"

struct aiString;

namespace assimp_util {
    glm::vec4 toVec4(const aiColor4D& v);
    glm::vec3 toVec3(const aiVector3D& v);
    glm::vec2 toVec2(const aiVector2D& v);
    glm::mat4 toMat4(const aiMatrix4x4& v);

    util::UVTransform toUVTransform(const aiUVTransform& src);

    glm::quat toQuat(const aiQuaternion& v);

    std::string resolvePath(
        std::string rootDir,
        std::string meshPath);

    std::string normalizeName(const aiString& name);
    std::string normalizeName(const std::string& name);
}

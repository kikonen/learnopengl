#pragma once

#include <assimp/types.h>

#include "assimp_util.h"
#include <util/util.h>
#include "util/file.h"

namespace {
    const std::string SUPPORTED_TYPES[]{
        "",
        ".glb",
        ".gltf",
        ".fbx",
        ".dae",
        ".obj",
    };
}

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

    std::string resolvePath(
        std::string rootDir,
        std::string meshPath)
    {
        // NOTE KI process in priority order; stop at first match
        for (const auto& ext : SUPPORTED_TYPES) {
            std::string filePath = util::joinPathExt(
                rootDir,
                meshPath,
                ext);

            if (util::fileExists(filePath)) {
                return filePath;
            }
        }

        return {};
    }

    std::string normalizeName(const aiString& name)
    {
        return normalizeName(name.C_Str());
    }

    std::string normalizeName(const std::string& name)
    {
        const std::regex RE_SPACE{ " " };

        return std::regex_replace(name, RE_SPACE, "-");
    }
}

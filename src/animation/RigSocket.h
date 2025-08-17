#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util/Transform.h"

namespace animation
{
    struct RigSocket
    {
        RigSocket(
            const std::string& name,
            const std::string& jointName,
            const util::Transform& offset,
            glm::vec3 meshScale);

        void updateTransforms();

        glm::mat4 calculateGlobalTransform(
            const glm::mat4& jointGlobalTransform) const;

        const std::string m_name;
        const std::string m_jointName;

        util::Transform m_offset;
        glm::vec3 m_meshScale;

        int16_t m_index{ -1 };
        int16_t m_jointIndex{ -1 };

    private:
        glm::mat4 m_offsetTransform;

        // NOTE KI or scaling mesh attached into socket into
        // scale of mesh owning socket, so that transforms happen
        // in correct scale
        glm::mat4 m_meshScaleTransform;
        glm::mat4 m_invMeshScaleTransform;
    };
}

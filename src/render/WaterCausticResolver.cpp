#include "WaterCausticResolver.h"

#include <algorithm>
#include <limits>

#include <glm/glm.hpp>

#include "asset/AABB.h"

#include "component/CameraComponent.h"

#include "model/Node.h"
#include "model/NodeType.h"
#include "model/Snapshot.h"

#include "render/Camera.h"
#include "render/NodeCollection.h"

#include "scene/Scene.h"

namespace render
{
    WaterCausticResolver::Result WaterCausticResolver::resolve(Scene* scene)
    {
        Result result;

        if (!scene) return result;

        auto* cameraNode = scene->getActiveCameraNode();
        if (!cameraNode || !cameraNode->m_camera) return result;

        auto* collection = scene->getCollection();
        if (!collection) return result;

        const glm::vec3 cameraPos = cameraNode->m_camera->getCamera().getWorldPosition();

        for (const auto& handle : collection->m_waterNodes) {
            auto* node = handle.toNode();
            if (!node) continue;

            auto* type = node->getType();
            // Without a configured depth there is no water volume to be inside.
            if (!type || type->m_waterDepth <= 0.f) continue;

            const auto* snapshot = node->getSnapshotRT();
            if (!snapshot) continue;

            const AABB& localAabb = type->getAABB();
            const glm::mat4& modelMatrix = snapshot->getModelMatrix();

            const glm::vec3 lmin = localAabb.getMin();
            const glm::vec3 lmax = localAabb.getMax();

            float minX = std::numeric_limits<float>::infinity();
            float maxX = -minX;
            float minZ = minX;
            float maxZ = -minX;

            for (int i = 0; i < 8; ++i) {
                glm::vec3 corner(
                    (i & 1) ? lmax.x : lmin.x,
                    (i & 2) ? lmax.y : lmin.y,
                    (i & 4) ? lmax.z : lmin.z);
                glm::vec3 world = glm::vec3(modelMatrix * glm::vec4(corner, 1.f));
                minX = std::min(minX, world.x);
                maxX = std::max(maxX, world.x);
                minZ = std::min(minZ, world.z);
                maxZ = std::max(maxZ, world.z);
            }

            const float surfaceY = snapshot->getWorldPosition().y;
            const float bottomY = surfaceY - type->m_waterDepth;

            const bool inside =
                cameraPos.x >= minX && cameraPos.x <= maxX &&
                cameraPos.z >= minZ && cameraPos.z <= maxZ &&
                cameraPos.y <= surfaceY && cameraPos.y >= bottomY;

            if (inside) {
                result.enabled = true;
                result.surfaceY = surfaceY;
                return result;
            }
        }

        return result;
    }
}

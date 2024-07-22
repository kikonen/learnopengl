#include "MeshGenerator.h"

#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/Log.h"
#include "util/Util.h"
#include "util/glm_util.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/ModelMesh.h"

#include "PhysicsEngine.h"

namespace {
}

namespace physics {
    MeshGenerator::MeshGenerator(const PhysicsEngine& engine)
        : m_engine{ engine}
    {}

    std::vector<std::unique_ptr<mesh::Mesh>> MeshGenerator::generateMeshes() const
    {
        std::vector<std::unique_ptr<mesh::Mesh>> meshes;
        meshes.reserve(m_engine.m_objects.size());

        for (const auto& obj : m_engine.m_objects) {
            if (!obj.m_geomId) continue;

            auto mesh = generateObject(obj);
            if (mesh) {
                meshes.push_back(std::move(mesh));
            }
        }

        return meshes;
    }

    std::unique_ptr<mesh::Mesh> MeshGenerator::generateObject(const Object& obj) const
    {
        const auto geomId = obj.m_geomId;
        if (!geomId) return nullptr;

        glm::vec3 pos{ 0.f };
        glm::quat rot{ 0.f, 0.f, 0.f, 1.f };

        std::unique_ptr<mesh::Mesh> mesh;
        {
            mesh::PrimitiveGenerator generator;
            switch (obj.m_geom.type) {
            case GeomType::plane: {
                dVector4 result;
                dGeomPlaneGetParams(geomId, result);
                const glm::vec4 plane{
                    static_cast<float>(result[0]),
                    static_cast<float>(result[1]),
                    static_cast<float>(result[2]),
                    static_cast<float>(result[3]) };
                glm::vec3 normal{ plane };
                float dist = plane.w;

                rot = util::normalToRotation(normal);
                glm::vec3 degrees = util::quatToDegrees(rot);

                KI_INFO_OUT(fmt::format(
                    "GET_PLANE: n={}, d={}, rot={}, degrees={}",
                    normal, dist, rot, degrees));

                pos = normal * dist;
                glm::vec2 size{ 50.f, 50.f };
                mesh = generator.generatePlane("<obj-plane>", size);
                break;
            }
            case GeomType::box: {
                dVector3 lengths;
                dGeomBoxGetLengths(geomId, lengths);
                glm::vec3 size{
                    static_cast<float>(lengths[0]) / 2.f,
                    static_cast<float>(lengths[1]) / 2.f,
                    static_cast<float>(lengths[2]) / 2.f
                };
                mesh = generator.generateBox(
                    "<geom-box>",
                    size);
                break;
            }
            case GeomType::sphere: {
                dReal radius = dGeomSphereGetRadius(geomId);
                mesh = generator.generateSphere(
                    "<geom-sphere>",
                    static_cast<float>(radius),
                    16,
                    8);
                break;
            }
            case GeomType::capsule: {
                dReal radius;
                dReal length;
                dGeomCapsuleGetParams(geomId, &radius, &length);
                mesh = generator.generateCapsule(
                    "<geom-capsule>",
                    static_cast<float>(radius),
                    static_cast<float>(length),
                    8,
                    8);
                break;
            }
            case GeomType::cylinder: {
                dReal radius;
                dReal length;
                dGeomCylinderGetParams(geomId, &radius, &length);
                mesh = generator.generateCylinder(
                    "<geom-cylinder>",
                    static_cast<float>(radius),
                    static_cast<float>(length),
                    8,
                    8);
                break;
            }
            }
        }

        if (mesh && obj.m_geom.type != GeomType::plane) {
            const dReal* dpos = dGeomGetPosition(geomId);
            const dReal* dquat = dGeomGetRotation(geomId);

            pos = glm::vec3{
                static_cast<float>(dpos[0]),
                static_cast<float>(dpos[1]),
                static_cast<float>(dpos[2]) };

            rot = glm::quat{
                static_cast<float>(dquat[0]),
                static_cast<float>(dquat[1]),
                static_cast<float>(dquat[2]),
                static_cast<float>(dquat[3]) };
        }

        glm::mat4 transform = glm::translate(glm::mat4{ 1.f }, pos) *
            glm::mat4(rot);

        for (auto& vertex : mesh->m_vertices) {
            vertex.pos = transform * glm::vec4(vertex.pos, 1.f);
        }

        return mesh;
    }
}

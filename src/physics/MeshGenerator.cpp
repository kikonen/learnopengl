#include "MeshGenerator.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/ModelMesh.h"

#include "PhysicsEngine.h"

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

        std::unique_ptr<mesh::Mesh> mesh;
        {
            mesh::PrimitiveGenerator generator;
            switch (obj.m_geom.type) {
            case GeomType::plane: {
                dVector4 result;
                dGeomPlaneGetParams(geomId, result);
                mesh = generator.generateQuad("<obj-box>");
                break;
            }
            case GeomType::box: {
                dVector3 lengths;
                dGeomBoxGetLengths(geomId, lengths);
                glm::vec3 size{
                    static_cast<float>(lengths[0]),
                    static_cast<float>(lengths[1]),
                    static_cast<float>(lengths[2])
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
                    static_cast<float>(radius));
                break;
            }
            case GeomType::capsule: {
                dReal radius;
                dReal length;
                dGeomCapsuleGetParams(geomId, &radius, &length);
                mesh = generator.generateCapsule(
                    "<geom-capsule>",
                    static_cast<float>(radius),
                    static_cast<float>(length));
                break;
            }
            case GeomType::cylinder: {
                dReal radius;
                dReal length;
                dGeomCylinderGetParams(geomId, &radius, &length);
                mesh = generator.generateCylinder(
                    "<geom-cylinder>",
                    static_cast<float>(radius),
                    static_cast<float>(length));
                break;
            }
            }
        }

        if (obj.m_geom.type != GeomType::plane) {
            const dReal* dpos = dGeomGetPosition(geomId);
            const dReal* dquat = dGeomGetRotation(geomId);

            glm::vec3 pos{
                static_cast<float>(dpos[0]),
                static_cast<float>(dpos[1]),
                static_cast<float>(dpos[2]) };

            const glm::quat rot{
                static_cast<float>(dquat[0]),
                static_cast<float>(dquat[1]),
                static_cast<float>(dquat[2]),
                static_cast<float>(dquat[3]) };
        }

        return mesh;
    }
}

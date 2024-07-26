#include "MeshGenerator.h"

#include <cmath>
#include <map>
#include <mutex>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/Log.h"
#include "util/Util.h"
#include "util/glm_util.h"


#include "render/DebugContext.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/ModelMesh.h"

#include "registry/MaterialRegistry.h"

#include "PhysicsEngine.h"

namespace {
    std::mutex g_materialLock;
    std::map<physics::GeomType, Material> g_materials;

    void setupMaterials()
    {
        std::lock_guard lock{ g_materialLock };

        if (g_materials.empty()) {
            const auto& matRed = Material::createMaterial(BasicMaterial::red);
            const auto& matWhite = Material::createMaterial(BasicMaterial::white);
            const auto& matBlue = Material::createMaterial(BasicMaterial::blue);
            const auto& matGreen = Material::createMaterial(BasicMaterial::green);
            const auto& matGold = Material::createMaterial(BasicMaterial::gold);

            g_materials.insert({
                { physics::GeomType::none, matWhite },
                { physics::GeomType::ray, matRed },
                { physics::GeomType::plane, matBlue },
                { physics::GeomType::box, matGreen },
            });

            for (auto& [geom, material] : g_materials) {
                MaterialRegistry::get().registerMaterial(material);
            }
        }
    }

    const Material& getMaterial(physics::GeomType type)
    {
        setupMaterials();
        {
            const auto& it = g_materials.find(type);
            if (it != g_materials.end()) return it->second;
        }
        {
            const auto& it = g_materials.find(physics::GeomType::none);
            return it->second;
        }
    }
}

namespace physics {
    MeshGenerator::MeshGenerator(const PhysicsEngine& engine)
        : m_engine{ engine}
    {}

    std::shared_ptr<std::vector<std::unique_ptr<mesh::Mesh>>> MeshGenerator::generateMeshes() const
    {
        if (!render::DebugContext::get().m_physicsShowObjects) return nullptr;

        auto meshes = std::make_shared<std::vector<std::unique_ptr<mesh::Mesh>>>();
        meshes->reserve(m_engine.m_objects.size());

        for (const auto& obj : m_engine.m_objects) {
            if (!obj.m_geomId) continue;

            auto mesh = generateObject(obj);
            if (mesh) {
                mesh->setMaterial(getMaterial(obj.m_geom.type));
                meshes->push_back(std::move(mesh));
            }
        }

        return meshes;
    }

    std::unique_ptr<mesh::Mesh> MeshGenerator::generateObject(const Object& obj) const
    {
        const auto geomId = obj.m_geomId;
        if (!geomId) return nullptr;

        glm::vec3 pos{ 0.f };
        glm::quat rot{ 1.f, 0.f, 0.f, 0.f };

        std::unique_ptr<mesh::Mesh> mesh;
        {
            switch (obj.m_geom.type) {
            case GeomType::ray: {
                dVector3 startOde;
                dVector3 dirOde;

                dGeomRayGet(geomId, startOde, dirOde);
                float length = static_cast<float>(dGeomRayGetLength(geomId));

                const glm::vec3 origin{
                    static_cast<float>(startOde[0]),
                    static_cast<float>(startOde[1]),
                    static_cast<float>(startOde[2]) };

                const glm::vec3 dir{
                    static_cast<float>(dirOde[0]),
                    static_cast<float>(dirOde[1]),
                    static_cast<float>(dirOde[2]) };

                auto generator = mesh::PrimitiveGenerator::ray();
                generator.name = fmt::format("<ray-{}>", obj.m_id);
                generator.origin = origin;
                generator.dir = dir;
                generator.length = length;
                mesh = generator.create();

                break;
            }
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

                rot = util::normalToRotation(normal, glm::vec3{0, 1.f, 0});

                //glm::vec3 degrees = util::quatToDegrees(rot);
                //KI_INFO_OUT(fmt::format(
                //    "GET_PLANE: n={}, d={}, rot={}, degrees={}",
                //    normal, dist, rot, degrees));

                pos = normal * dist;
                glm::vec2 size{ 100.f, 100.f };

                auto generator = mesh::PrimitiveGenerator::plane();
                generator.name = fmt::format("<plane-{}>", obj.m_id);
                generator.size = glm::vec3{ size.x, size.y, 0.f };
                mesh = generator.create();

                break;
            }
            case GeomType::box: {
                dVector3 lengths;
                dGeomBoxGetLengths(geomId, lengths);
                glm::vec3 size{
                    static_cast<float>(lengths[0]) * 0.5f,
                    static_cast<float>(lengths[1]) * 0.5f,
                    static_cast<float>(lengths[2]) * 0.5f
                };

                auto generator = mesh::PrimitiveGenerator::box();
                generator.name = fmt::format("<box-{}>", obj.m_id);
                generator.size = size;
                mesh = generator.create();

                break;
            }
            case GeomType::sphere: {
                dReal radius = dGeomSphereGetRadius(geomId);

                auto generator = mesh::PrimitiveGenerator::sphere();
                generator.name = fmt::format("<sphere-{}>", obj.m_id);
                generator.radius = static_cast<float>(radius);
                generator.slices = 16;
                generator.segments = 8;
                mesh = generator.create();

                break;
            }
            case GeomType::capsule: {
                dReal radius;
                dReal length;
                dGeomCapsuleGetParams(geomId, &radius, &length);

                auto generator = mesh::PrimitiveGenerator::capsule();
                generator.name = fmt::format("<capsule-{}>", obj.m_id);
                generator.radius = static_cast<float>(radius);
                generator.length = static_cast<float>(length * 0.5f);
                generator.slices = 8;
                generator.segments = 4;
                mesh = generator.create();

                break;
            }
            case GeomType::cylinder: {
                dReal radius;
                dReal length;
                dGeomCylinderGetParams(geomId, &radius, &length);

                auto generator = mesh::PrimitiveGenerator::capped_cylinder();
                generator.name = fmt::format("<cylinder-{}>", obj.m_id);
                generator.radius = static_cast<float>(radius);
                generator.length = static_cast<float>(length * 0.5f);
                generator.slices = 8;
                generator.segments = 4;
                mesh = generator.create();

                break;
            }
            }
        }

        if (mesh && obj.m_bodyId && obj.m_geom.type != GeomType::plane) {
            const dReal* dpos = dBodyGetPosition(obj.m_bodyId);
            const dReal* dquat = dBodyGetQuaternion(obj.m_bodyId);

            pos = glm::vec3{
                static_cast<float>(dpos[0]),
                static_cast<float>(dpos[1]),
                static_cast<float>(dpos[2]) };

            rot = glm::quat{
                static_cast<float>(dquat[0]),
                static_cast<float>(dquat[1]),
                static_cast<float>(dquat[2]),
                static_cast<float>(dquat[3]) };

            //if (obj.m_geom.type == GeomType::box) {
            //    auto degrees = util::quatToDegrees(rot);

            //    KI_INFO_OUT(fmt::format(
            //        "GET_GEOM: id={}, type={}, pos={}, rot={}, degrees={}",
            //        obj.m_id, util::as_integer(obj.m_geom.type), pos, rot, degrees));
            //}
        }

        if (mesh) {
            glm::mat4 transform = glm::translate(glm::mat4{ 1.f }, pos) *
                glm::mat4(rot);

            for (auto& vertex : mesh->m_vertices) {
                vertex.pos = transform * glm::vec4(vertex.pos, 1.f);
            }
        }

        return mesh;
    }
}

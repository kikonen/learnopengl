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

#include "asset/Material.h"

#include "render/DebugContext.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/Mesh.h"
#include "mesh/MeshInstance.h"

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
            const auto& matYellow = Material::createMaterial(BasicMaterial::yellow);
            const auto& matGold = Material::createMaterial(BasicMaterial::gold);

            g_materials.insert({
                { physics::GeomType::none, matWhite },
                { physics::GeomType::ray, matRed },
                { physics::GeomType::plane, matBlue },
                { physics::GeomType::height_field, matYellow },
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

    void MeshGenerator::clear()
    {
        m_cache.clear();
    }

    std::shared_ptr<std::vector<mesh::MeshInstance>> MeshGenerator::generateMeshes()
    {
        if (!render::DebugContext::get().m_physicsShowObjects) return nullptr;

        auto meshes = std::make_shared<std::vector<mesh::MeshInstance>>();
        meshes->reserve(m_engine.m_objects.size());

        for (const auto& obj : m_engine.m_objects) {
            if (!obj.m_geom.physicId) continue;

            auto instance = generateObject(obj);
            if (instance.m_mesh) {
                instance.m_materialIndex = getMaterial(obj.m_geom.type).m_registeredIndex;
                meshes->push_back(instance);
            }
        }

        return meshes;
    }

    mesh::MeshInstance MeshGenerator::generateObject(const Object& obj)
    {
        const auto geomId = obj.m_geom.physicId;
        if (!geomId) return {};

        glm::vec3 pos{ 0.f };
        glm::quat rot{ 1.f, 0.f, 0.f, 0.f };
        glm::vec3 offset{ 0.f };

        std::string cacheKey;

        std::shared_ptr<mesh::Mesh> mesh;
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

                cacheKey = fmt::format(
                    "ray-{}-{}-{}",
                    origin, dir, length);

                auto generator = mesh::PrimitiveGenerator::ray();
                generator.name = cacheKey;
                generator.origin = origin;
                generator.dir = dir;
                generator.length = length;

                mesh = generator.create();
                cacheKey = "";

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

                cacheKey = fmt::format(
                    "plane-{}",
                    size);

                auto generator = mesh::PrimitiveGenerator::plane();
                generator.name = cacheKey;
                generator.size = glm::vec3{ size.x, size.y, 0.f };

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    mesh = saveMesh(cacheKey, generator.create());
                }

                break;
            }
            case GeomType::height_field: {
                const auto* heightMap = m_engine.getHeightMap(obj.m_heightMapId);
                if (heightMap) {
                    //dxHeightfieldData& data = *dGeomHeightfieldGetHeightfieldData(geomId);

                    cacheKey = fmt::format(
                        "height-{}",
                        heightMap->m_id);

                    auto generator = mesh::PrimitiveGenerator::height_field();
                    generator.name = cacheKey;
                    generator.heightData = heightMap->m_heightData;
                    generator.size = {
                        heightMap->m_worldSizeU,
                        0.f,
                        heightMap->m_worldSizeV,
                    };
                    generator.heightSamplesWidth = heightMap->m_dataWidth;
                    generator.heightSamplesDepth = heightMap->m_dataDepth;
                    generator.p = 8;
                    generator.q = 8;

                    offset = { -generator.size.x * 0.5f, 0.f, -generator.size.z * 0.5f };

                    mesh = findMesh(cacheKey);
                    if (!mesh) {
                        mesh = saveMesh(cacheKey, generator.create());
                    }
                }
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

                cacheKey = fmt::format(
                    "box-{}",
                    size);

                auto generator = mesh::PrimitiveGenerator::box();
                generator.name = cacheKey;
                generator.size = size;

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    mesh = saveMesh(cacheKey, generator.create());
                }

                break;
            }
            case GeomType::sphere: {
                dReal radius = dGeomSphereGetRadius(geomId);

                cacheKey = fmt::format(
                    "sphere-{}",
                    radius);

                auto generator = mesh::PrimitiveGenerator::sphere();
                generator.name = cacheKey;
                generator.radius = static_cast<float>(radius);
                generator.slices = 16;
                generator.segments = { 8, 0, 0 };

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    mesh = saveMesh(cacheKey, generator.create());
                }

                break;
            }
            case GeomType::capsule: {
                dReal radius;
                dReal length;
                dGeomCapsuleGetParams(geomId, &radius, &length);

                cacheKey = fmt::format(
                    "capsule-{}-{}",
                    radius, length);

                auto generator = mesh::PrimitiveGenerator::capsule();
                generator.name = cacheKey;
                generator.radius = static_cast<float>(radius);
                generator.length = static_cast<float>(length * 0.5f);
                generator.slices = 8;
                generator.segments = { 4, 0, 0 };

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    mesh = saveMesh(cacheKey, generator.create());
                }

                break;
            }
            case GeomType::cylinder: {
                dReal radius;
                dReal length;
                dGeomCylinderGetParams(geomId, &radius, &length);

                cacheKey = fmt::format(
                    "cylinder-{}-{}",
                    radius, length);

                auto generator = mesh::PrimitiveGenerator::capped_cylinder();
                generator.name = cacheKey;
                generator.radius = static_cast<float>(radius);
                generator.length = static_cast<float>(length * 0.5f);
                generator.slices = 8;
                generator.segments = { 4, 0, 0 };

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    mesh = saveMesh(cacheKey, generator.create());
                }

                break;
            }
            }
        }

        if (mesh && obj.m_geom.physicId && obj.m_geom.type != GeomType::plane) {
            pos = obj.m_geom.getPhysicPosition();
            rot = obj.m_geom.getPhysicRotation();

            pos += offset;

            //if (obj.m_geom.type == GeomType::box) {
            //    auto degrees = util::quatToDegrees(rot);

            //    KI_INFO_OUT(fmt::format(
            //        "GET_GEOM: id={}, type={}, pos={}, rot={}, degrees={}",
            //        obj.m_id, util::as_integer(obj.m_geom.type), pos, rot, degrees));
            //}
        }

        //if (mesh && obj.m_body.physicId) {
        //    pos = obj.m_body.getPhysicPosition();
        //    rot = obj.m_body.getPhysicRotation();
        //}

        glm::mat4 transform = glm::translate(glm::mat4{ 1.f }, pos) *
            glm::mat4(rot);

        return { !cacheKey.empty(), mesh, transform};
    }

    std::shared_ptr<mesh::Mesh> MeshGenerator::findMesh(const std::string& key)
    {
        const auto& it = m_cache.find(key);
        if (it != m_cache.end()) return it->second;
        return nullptr;
    }

    std::shared_ptr<mesh::Mesh> MeshGenerator::saveMesh(
        const std::string& key,
        std::shared_ptr<mesh::Mesh> mesh)
    {
        m_cache.insert({ key, mesh });
        return mesh;
    }
}

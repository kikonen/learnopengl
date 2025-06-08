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
#include "util/util.h"
#include "util/glm_util.h"

#include "material/Material.h"

#include "model/Node.h"

#include "render/DebugContext.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/Mesh.h"
#include "mesh/PrimitiveMesh.h"
#include "mesh/MeshInstance.h"

#include "PhysicsSystem.h"
#include "ode_util.h"


namespace {
    constexpr float SCALE = 1.0f;
    constexpr float OFFSET = 0.0f;

    std::mutex g_materialLock;
    std::map<physics::GeomType, Material> g_materials;

    void setupMaterials()
    {
        std::lock_guard lock{ g_materialLock };

        if (g_materials.empty()) {
            const auto matRed = Material::createMaterial(BasicMaterial::red);
            const auto matWhite = Material::createMaterial(BasicMaterial::white);
            const auto matBlue = Material::createMaterial(BasicMaterial::blue);
            const auto matGreen = Material::createMaterial(BasicMaterial::green);
            const auto matYellow = Material::createMaterial(BasicMaterial::yellow);
            const auto matGold = Material::createMaterial(BasicMaterial::gold);

            g_materials.insert({
                { physics::GeomType::none, matWhite },
                { physics::GeomType::ray, matRed },
                { physics::GeomType::plane, matBlue },
                { physics::GeomType::height_field, matYellow },
                { physics::GeomType::box, matGreen },
            });

            for (auto& [geom, material] : g_materials) {
                material.registerMaterial();
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
    MeshGenerator::MeshGenerator(const PhysicsSystem& physicsSystem)
        : m_physicsSystem{ physicsSystem }
    {}

    void MeshGenerator::clear()
    {
        m_cache.clear();
    }

    std::shared_ptr<std::vector<mesh::MeshInstance>> MeshGenerator::generateMeshes(bool onlyNavMesh)
    {
        const auto spaceId = m_physicsSystem.m_spaceId;
        auto geomCount = dSpaceGetNumGeoms(spaceId);

        auto meshes = std::make_shared<std::vector<mesh::MeshInstance>>();
        meshes->reserve(geomCount);

        for (int i = 0; i < geomCount; i++) {
            auto geomId = dSpaceGetGeom(spaceId, i);
            auto geomType = getGeomType(dGeomGetClass(geomId));

            bool isNavMesh = false;
            {
                auto objectId = (physics::object_id)dGeomGetData(geomId);
                const auto* node = m_physicsSystem.getNodeHandle(objectId).toNode();
                if (node) {
                    isNavMesh = node->m_typeFlags.navPhysics;
                }
            }

            if (!onlyNavMesh || isNavMesh) {
                auto instance = generateMesh(geomType, geomId);
                if (instance.m_mesh) {
                    instance.m_materialIndex = getMaterial(geomType).m_registeredIndex;
                    meshes->push_back(instance);
                }
            }
        }

        return meshes;
    }

    mesh::MeshInstance MeshGenerator::generateMesh(
        physics::GeomType geomType,
        dGeomID geomId)
    {
        if (geomType == physics::GeomType::none) return {};

        glm::vec3 pos{ 0.f };
        glm::quat rot{ 1.f, 0.f, 0.f, 0.f };
        glm::vec3 offset{ 0.f };

        std::string cacheKey;

        std::shared_ptr<mesh::Mesh> mesh;
        {
            switch (geomType) {
            case physics::GeomType::ray: {
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
            case physics::GeomType::plane: {
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

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::plane();
                    generator.name = cacheKey;
                    generator.size = glm::vec3{ size.x, size.y, 0.f };
                    mesh = saveMesh(cacheKey, generator.create());
                }

                break;
            }
            case physics::GeomType::height_field: {
                auto heightDataId = dGeomHeightfieldGetHeightfieldData(geomId);
                const auto* heightMap = m_physicsSystem.getHeightMap(heightDataId);
                if (heightMap) {
                    //dxHeightfieldData& data = *dGeomHeightfieldGetHeightfieldData(geomId);

                    cacheKey = fmt::format(
                        "height-{}",
                        heightMap->m_id);

                    mesh = findMesh(cacheKey);
                    if (!mesh) {
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
                        mesh = saveMesh(cacheKey, generator.create());
                    }

                    offset = { -heightMap->m_worldSizeU * 0.5f, 0.f + OFFSET, -heightMap->m_worldSizeV * 0.5f };
                }
                break;
            }
            case physics::GeomType::box: {
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


                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::box();
                    generator.name = cacheKey;
                    generator.size = size * SCALE;
                    mesh = saveMesh(cacheKey, generator.create());
                }

                break;
            }
            case physics::GeomType::sphere: {
                dReal radius = dGeomSphereGetRadius(geomId);

                cacheKey = fmt::format(
                    "sphere-{}",
                    radius);

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::sphere();
                    generator.name = cacheKey;
                    generator.radius = static_cast<float>(radius) * SCALE;
                    generator.slices = 16;
                    generator.segments = { 8, 0, 0 };
                    mesh = saveMesh(cacheKey, generator.create());
                }

                break;
            }
            case physics::GeomType::capsule: {
                dReal radius;
                dReal length;
                dGeomCapsuleGetParams(geomId, &radius, &length);

                cacheKey = fmt::format(
                    "capsule-{}-{}",
                    radius, length);


                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::capsule();
                    generator.name = cacheKey;
                    generator.radius = static_cast<float>(radius) * SCALE;
                    generator.length = static_cast<float>(length * 0.5f) * SCALE;
                    generator.slices = 8;
                    generator.segments = { 4, 0, 0 };
                    mesh = saveMesh(cacheKey, generator.create());
                }

                break;
            }
            case physics::GeomType::cylinder: {
                dReal radius;
                dReal length;
                dGeomCylinderGetParams(geomId, &radius, &length);

                cacheKey = fmt::format(
                    "cylinder-{}-{}",
                    radius, length);

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::capped_cylinder();
                    generator.name = cacheKey;
                    generator.radius = static_cast<float>(radius) * SCALE;
                    generator.length = static_cast<float>(length * 0.5f) * SCALE;
                    generator.slices = 8;
                    generator.segments = { 4, 0, 0 };
                    mesh = saveMesh(cacheKey, generator.create());
                }

                break;
            }
            }
        }

        if (mesh && geomId
            && geomType != physics::GeomType::ray
            && geomType != physics::GeomType::plane)
        {
            pos = getPhysicPosition(geomId);
            rot = getPhysicRotation(geomId);

            pos += offset;
        }

        glm::mat4 transform = glm::translate(glm::mat4{ 1.f }, pos) *
            glm::mat4(rot);

        backend::DrawOptions drawOptions;
        if (mesh) {
            auto* primitiveMesh = dynamic_cast<mesh::PrimitiveMesh*>(mesh.get());

            primitiveMesh->setupVertexCounts();

            drawOptions.m_mode = mesh->getDrawMode();
            drawOptions.m_type = backend::DrawOptions::Type::elements;
            drawOptions.m_lineMode = true;
            drawOptions.m_renderBack = geomType == GeomType::plane || geomType == GeomType::height_field;
        }

        return { transform, mesh, drawOptions, -1, 0, !cacheKey.empty() };
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

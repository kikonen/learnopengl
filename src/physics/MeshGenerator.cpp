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

#include "asset/SphereVolume.h"

#include "material/Material.h"

#include "model/Node.h"

#include "debug/DebugContext.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/Mesh.h"
#include "mesh/PrimitiveMesh.h"
#include "mesh/Transform.h"
#include "mesh/MeshInstance.h"

#include "PhysicsSystem.h"
#include "ode_util.h"


namespace {
    constexpr float SCALE = 1.0f;
    constexpr float OFFSET = 0.0f;

    constexpr float PLANE_SIZE = 100.f;

    const glm::vec3 UP{ 0.f, 1.f, 0.f };

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

    void clearMaterials()
    {
        std::lock_guard lock{ g_materialLock };

        g_materials.clear();
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

    MeshGenerator::~MeshGenerator()
    {
        clear();
    }

    void MeshGenerator::clear()
    {
        m_cache.clear();
        clearMaterials();
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
        glm::vec3 scale{ 1.f };
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

                pos = glm::vec3{
                    static_cast<float>(startOde[0]),
                    static_cast<float>(startOde[1]),
                    static_cast<float>(startOde[2]) };

                const glm::vec3 normal{
                    static_cast<float>(dirOde[0]),
                    static_cast<float>(dirOde[1]),
                    static_cast<float>(dirOde[2]) };

                rot = util::normalToQuat(normal, UP);
                scale = glm::vec3{ length };

                cacheKey = fmt::format(
                    "ray-{}",
                    1);

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::ray();
                    generator.name = cacheKey;
                    generator.origin = glm::vec3{ 0 };
                    generator.dir = UP;
                    generator.length = length;
                    mesh = saveMesh(cacheKey, generator.create());
                }

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

                rot = util::normalToQuat(normal, glm::vec3{0, 1.f, 0});

                //glm::vec3 degrees = util::quatToDegrees(rot);
                //KI_INFO_OUT(fmt::format(
                //    "GET_PLANE: n={}, d={}, rot={}, degrees={}",
                //    normal, dist, rot, degrees));

                pos = normal * dist;
                scale = glm::vec3{ PLANE_SIZE, PLANE_SIZE, PLANE_SIZE };

                cacheKey = fmt::format(
                    "plane-{}",
                    1);

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::plane();
                    generator.name = cacheKey;
                    generator.size = glm::vec3{ 1.f, 1.f, 0.f };
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

                scale = size;
                cacheKey = fmt::format(
                    "box-{}",
                    1);

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::box();
                    generator.name = cacheKey;
                    generator.size = glm::vec3{ 1.f };
                    mesh = saveMesh(cacheKey, generator.create());
                }

                break;
            }
            case physics::GeomType::sphere: {
                dReal dRadius = dGeomSphereGetRadius(geomId);
                float radius = static_cast<float>(dRadius);

                scale = glm::vec3{ radius };

                cacheKey = fmt::format(
                    "sphere-{}",
                    1);

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::sphere();
                    generator.name = cacheKey;
                    generator.radius = 1.f;
                    generator.slices = 16;
                    generator.segments = { 8, 0, 0 };
                    mesh = saveMesh(cacheKey, generator.create());
                }

                break;
            }
            case physics::GeomType::capsule: {
                dReal dRadius;
                dReal dLength;
                dGeomCapsuleGetParams(geomId, &dRadius, &dLength);

                float radius = static_cast<float>(dRadius);
                float length = static_cast<float>(dLength);
                float ratio = 1.f / radius;

                scale = glm::vec3{1.f / ratio};

                cacheKey = fmt::format(
                    "capsule-{}-{}",
                    1, length * ratio);

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::capsule();
                    generator.name = cacheKey;
                    generator.radius = 1.f;
                    generator.length = 0.5f * length * ratio;
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

                scale = glm::vec3{
                    static_cast<float>(radius),
                    static_cast<float>(radius),
                    static_cast<float>(length)
                    };

                cacheKey = fmt::format(
                    "cylinder-{}-{}",
                    1, 0.5);

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::capped_cylinder();
                    generator.name = cacheKey;
                    generator.radius = 1.f;
                    generator.length = 0.5f;
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

        mesh::Transform transform;
        transform.setPosition(pos);
        transform.setRotation(rot);
        transform.setScale(scale);
        transform.updateMatrix();
        transform.updateWorldVolume();

        backend::DrawOptions drawOptions;
        if (mesh) {
            auto* primitiveMesh = dynamic_cast<mesh::PrimitiveMesh*>(mesh.get());

            primitiveMesh->setupVertexCounts();

            drawOptions.m_mode = mesh->getDrawMode();
            drawOptions.m_type = backend::DrawOptions::Type::elements;
            drawOptions.m_lineMode = true;
            drawOptions.m_renderBack = geomType == GeomType::plane || geomType == GeomType::height_field;
        }

        return {
            mesh.get(),
            transform.getMatrix(),
            transform.getWorldVolume(),
            drawOptions,
            -1,
            0,
            !cacheKey.empty()};
    }

    std::shared_ptr<mesh::Mesh> MeshGenerator::findMesh(const std::string& key)
    {
        const auto& it = m_cache.find(key);
        if (it != m_cache.end()) return it->second;
        return nullptr;
    }

    std::shared_ptr<mesh::Mesh> MeshGenerator::saveMesh(
        const std::string& key,
        const std::shared_ptr<mesh::Mesh>& mesh)
    {
        m_cache.insert({ key, mesh });
        return mesh;
    }
}

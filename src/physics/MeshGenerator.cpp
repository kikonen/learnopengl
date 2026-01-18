#include "MeshGenerator.h"

#include <cmath>
#include <map>
#include <mutex>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>

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
#include "jolt_util.h"
#include "JoltFoundation.h"
#include "HeightMap.h"


namespace {
    constexpr float SCALE = 1.0f;
    constexpr float OFFSET = 0.0f;

    constexpr float PLANE_SIZE = 1.f;

    const glm::vec3 UP{ 0.f, 1.f, 0.f };

    std::mutex g_materialLock;
    std::map<physics::ShapeType, Material> g_materials;

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
                { physics::ShapeType::none, matWhite },
                { physics::ShapeType::ray, matRed },
                { physics::ShapeType::plane, matBlue },
                { physics::ShapeType::height_field, matYellow },
                { physics::ShapeType::box, matGreen },
            });

            for (auto& [shapeType, material] : g_materials) {
                material.registerMaterial();
            }
        }
    }

    void clearMaterials()
    {
        std::lock_guard lock{ g_materialLock };

        g_materials.clear();
    }

    const Material& getMaterial(physics::ShapeType type)
    {
        setupMaterials();
        {
            const auto& it = g_materials.find(type);
            if (it != g_materials.end()) return it->second;
        }
        {
            const auto& it = g_materials.find(physics::ShapeType::none);
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
        auto meshes = std::make_shared<std::vector<mesh::MeshInstance>>();

        // Iterate over our physics objects
        const auto objectCount = m_physicsSystem.getObjectCount();
        meshes->reserve(objectCount);

        for (physics::object_id id = 1; id < objectCount; id++) {
            const auto* obj = m_physicsSystem.getObject(id);
            if (!obj) continue;
            if (!obj->ready()) continue;

            bool isNavMesh = false;
            {
                const auto* node = m_physicsSystem.getNodeHandle(id).toNode();
                if (node) {
                    isNavMesh = node->m_typeFlags.navPhysics;
                }
            }

            if (!onlyNavMesh || isNavMesh) {
                auto instance = generateMesh(*obj, id);
                if (instance.m_mesh) {
                    auto shapeType = obj->m_shape.type;
                    if (shapeType == ShapeType::none && obj->m_body.type != BodyType::none) {
                        // Use body type for material if shape type is none
                        switch (obj->m_body.type) {
                        case BodyType::box: shapeType = ShapeType::box; break;
                        case BodyType::sphere: shapeType = ShapeType::sphere; break;
                        case BodyType::capsule: shapeType = ShapeType::capsule; break;
                        case BodyType::cylinder: shapeType = ShapeType::cylinder; break;
                        default: break;
                        }
                    }
                    instance.m_materialIndex = getMaterial(shapeType).m_registeredIndex;
                    meshes->push_back(instance);
                }
            }
        }

        return meshes;
    }

    mesh::MeshInstance MeshGenerator::generateMesh(
        const physics::Object& obj,
        physics::object_id objectId)
    {
        const auto& body = obj.m_body;
        const auto& shape = obj.m_shape;

        // Determine which type to use (prefer shape, fall back to body)
        physics::ShapeType shapeType = shape.type;
        physics::BodyType bodyType = body.type;

        if (shapeType == ShapeType::none && bodyType == BodyType::none) {
            return {};
        }

        // Skip rays - they're not visualized via bodies
        if (shapeType == ShapeType::ray) {
            return {};
        }

        glm::vec3 pos{ 0.f };
        glm::vec3 scale = obj.m_scale;
        glm::quat rot{ 1.f, 0.f, 0.f, 0.f };
        glm::vec3 offset{ 0.f };

        std::string cacheKey;
        std::shared_ptr<mesh::Mesh> mesh;

        // Get position and rotation from Jolt body
        JPH::BodyID joltBodyId;
        if (body.hasPhysicsBody()) {
            joltBodyId = body.m_bodyId;
        } else if (shape.hasPhysicsBody()) {
            joltBodyId = shape.m_staticBodyId;
        }

        if (!joltBodyId.IsInvalid()) {
            const auto& bodyInterface = m_physicsSystem.getBodyInterface();
            pos = fromJolt(bodyInterface.GetPosition(joltBodyId));
            rot = fromJolt(bodyInterface.GetRotation(joltBodyId));
        }

        // Generate mesh based on type
        if (shapeType != ShapeType::none) {
            switch (shapeType) {
            case ShapeType::plane: {
                scale *= glm::vec3{ PLANE_SIZE, PLANE_SIZE, PLANE_SIZE };

                cacheKey = fmt::format("plane-{}", 1);

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::plane();
                    generator.name = cacheKey;
                    generator.size = glm::vec3{ 1.f, 1.f, 0.f };
                    mesh = saveMesh(cacheKey, generator.create());
                }
                break;
            }
            case ShapeType::height_field: {
                // Find the height map for this object
                for (physics::height_map_id hid = 1; hid < 100; hid++) {
                    const auto* heightMap = m_physicsSystem.getHeightMap(hid);
                    if (!heightMap) continue;

                    cacheKey = fmt::format("height-{}", heightMap->m_id);

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
                    break;
                }
                break;
            }
            case ShapeType::box: {
                scale *= shape.size;
                cacheKey = fmt::format("box-{}", 1);

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::box();
                    generator.name = cacheKey;
                    generator.size = glm::vec3{ 1.f };
                    mesh = saveMesh(cacheKey, generator.create());
                }
                break;
            }
            case ShapeType::sphere: {
                float radius = shape.size.x;
                scale *= glm::vec3{ radius };

                cacheKey = fmt::format("sphere-{}", 1);

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
            case ShapeType::capsule: {
                float radius = shape.size.x;
                float length = shape.size.y * 2.f;
                float ratio = 1.f / radius;

                scale *= glm::vec3{ 1.f / ratio };

                cacheKey = fmt::format("capsule-{}-{}", 1, length * ratio);

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
            case ShapeType::cylinder: {
                float radius = shape.size.x;
                float length = shape.size.y * 2.f;

                scale *= glm::vec3{ radius, radius, length };

                cacheKey = fmt::format("cylinder-{}-{}", 1, 0.5);

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
            default:
                break;
            }
        }
        else if (bodyType != BodyType::none) {
            // Use body type if shape type is none
            switch (bodyType) {
            case BodyType::box: {
                scale *= body.size;
                cacheKey = fmt::format("box-{}", 1);

                mesh = findMesh(cacheKey);
                if (!mesh) {
                    auto generator = mesh::PrimitiveGenerator::box();
                    generator.name = cacheKey;
                    generator.size = glm::vec3{ 1.f };
                    mesh = saveMesh(cacheKey, generator.create());
                }
                break;
            }
            case BodyType::sphere: {
                float radius = body.size.x;
                scale *= glm::vec3{ radius };

                cacheKey = fmt::format("sphere-{}", 1);

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
            case BodyType::capsule: {
                float radius = body.size.x;
                float length = body.size.y * 2.f;
                float ratio = 1.f / radius;

                scale *= glm::vec3{ 1.f / ratio };

                cacheKey = fmt::format("capsule-{}-{}", 1, length * ratio);

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
            case BodyType::cylinder: {
                float radius = body.size.x;
                float length = body.size.y * 2.f;

                scale *= glm::vec3{ radius, radius, length };

                cacheKey = fmt::format("cylinder-{}-{}", 1, 0.5);

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
            default:
                break;
            }
        }

        pos += offset;

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
            drawOptions.m_renderBack = shapeType == ShapeType::plane || shapeType == ShapeType::height_field;
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

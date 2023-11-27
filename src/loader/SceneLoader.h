#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include <ki/uuid.h>

#pragma warning(push)
#pragma warning(disable : 4251)
#pragma warning(disable : 4275)

#include <yaml-cpp/yaml.h>

#pragma warning(pop)

#include "asset/Material.h"
#include "asset/MaterialField.h"
#include "asset/Shader.h"
#include "asset/CustomMaterial.h"

#include "registry/EntityType.h"

namespace event {
    class Dispatcher;
}

namespace physics {
    class Object;
}

class Sprite;
struct Shape;

class Light;
class Camera;

class Registry;

class MeshType;
class Node;

class NodeGenerator;
class NodeController;

class AsyncLoader;

class SceneLoader
{
public:
    using BaseUUID = std::vector<std::string>;

private:
    struct MetaData {
        std::string name;

        std::string assetsDir;
        std::string modelsDir;
    };

    struct SkyboxData {
        std::string programName{ "skybox" };
        std::string materialName{};
        int priority{ -100 };

        bool gammaCorrect{ true };
        bool hdri{ false };
        bool swapFaces{ false };
        bool loadedFaces{ false };
        std::array<std::string, 6> faces;

        bool const valid() {
            return !materialName.empty();
        }
    };

    struct Repeat {
        int xCount{ 1 };
        int yCount{ 1 };
        int zCount{ 1 };

        double xStep{ 0.f };
        double yStep{ 0.f };
        double zStep{ 0.f };
    };

    struct Tiling {
        int tile_size{ 100 };
        glm::uvec3 tiles{ 1 };
        float height_scale{ 32 };
        float horizontal_scale{ 1 };
        glm::vec2 vertical_range{ 0, 32 };
    };

    enum class ControllerType {
        none,
        camera,
    };

    struct ControllerData {
        bool enabled{ false };
        ControllerType type{ ControllerType::none };

        int mode{ 0 };
        float speed{ 0.f };
    };

    enum class GeneratorType {
        none,
        grid,
        terrain,
        asteroid_belt,
    };

    struct GeneratorData {
        bool enabled{ false };
        GeneratorType type{ GeneratorType::none };

        int count{ 0 };
        int mode{ 0 };
        float radius{ 0.f };

        Repeat repeat;
        Tiling tiling;
    };

    struct CameraData {
        bool enabled{ false };

        bool isDefault { false };

        bool orthagonal{ false };
        std::array<float, 4> viewport{ 0.f };

        float fov{ 45.f };

        glm::vec3 front{ 0.f, 0.f, -1.f };
        glm::vec3 up{ 0.f, 1.f, 0.f };

        // pos relative to owning node
        glm::vec3 pos{ 0.f };
        glm::vec3 rotation{ 0.f };
    };

    enum class LightType {
        none,
        directional,
        point,
        spot
    };

    struct LightData {
        bool enabled{ false };
        LightType type{ LightType::none };

        glm::vec3 pos{ 0.f };

        BaseUUID targetIdBase;

        float linear{ 0.f };
        float quadratic{ 0.f };

        float cutoffAngle{ 0.f };
        float outerCutoffAngle{ 0.f };

        glm::vec3 diffuse{ 0.5f, 0.5f, 0.5f };
        float intensity{ 1.f };
    };

    enum class CustomMaterialType {
        none,
        text,
        skybox,
    };

    struct CustomMaterialData {
        CustomMaterialType type{ CustomMaterialType::none };

        std::string fontName;
        float fontSize;
    };

    enum class BodyType {
        none,
        sphere,
        box,
    };

    struct BodyData {
        BodyType type{ BodyType::none };

        // NOTE KI *SCALED* using scale of node
        // size{0] == radius
        glm::vec3 size{ 1.f };

        float mass{ 1.f };

        // initial values for physics
        glm::vec3 linearVel{ 0.f };
        glm::vec3 angularVel{ 0.f };

        // NOTE KI *ROTATED* using rotation of node
        // axis + angle
        //glm::vec4 quat{ 0.f };
        glm::vec3 rotation{ 0.f };
    };

    enum class GeomType {
        none,
        plane,
        sphere,
        box,
        capsule,
        cylinder,
    };

    struct GeomData {
        GeomType type{ GeomType::none };

        // NOTE KI *SCALED* using scale of node
        // size{0] == radius
        glm::vec3 size{ 1.f };

        glm::vec4 plane{ 0.f, 1.f, 0.f, 0.f };

        unsigned int category{ UINT_MAX };
        unsigned int collide{ UINT_MAX };
    };

    struct PhysicsData {
        bool enabled{ false };

        std::string space{ "default" };

        BodyData body;
        GeomData geom;
    };

    struct EntityCloneData {
        bool valid{ false };

        bool enabled{ false };

        EntityType type { EntityType::model };

        std::string name;
        std::string desc;

        int priority{ 0 };

        BaseUUID idBase;
        BaseUUID parentIdBase;

        std::string meshName;
        std::string meshPath;

        std::string programName{};
        std::string geometryType;

        std::string depthProgramName{ SHADER_DEPTH_PASS };

        std::map<std::string, std::string> programDefinitions{};
        std::unordered_map<std::string, bool> renderFlags{};
        glm::vec3 position{ 0.f };
        glm::vec3 rotation{ 0.f };
        glm::vec3 front{ 0.f, 0.f, 1.f };
        glm::vec3 scale{ 1.f };

        bool selected{ false };
        bool instanced{ false };
        bool cloneMesh{ true };

        Tiling tiling;
        glm::uvec3 tile{ 0 };

        glm::vec3 clonePositionOffset{ 0.f };

        bool loadTextures{ true };
        std::string materialName;
        // NOTE KI overrides *ALL* materials with defaultMaterial
        bool forceMaterial{ false };

        MaterialField materialModifierFields;
        Material materialModifiers;
        bool materialModifiers_enabled = false;

        CustomMaterialData customMaterial;
        PhysicsData physics;

        std::string spriteName;

        int batchSize{ -1 };

        Repeat repeat;

        std::string script;

        ControllerData controller;
        CameraData camera;
        LightData light;

        GeneratorData generator;
    };

    struct EntityData {
        bool isRoot{ false };
        EntityCloneData base;
        std::vector<EntityCloneData> clones;
    };

public:
    SceneLoader(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive,
        std::shared_ptr<AsyncLoader> asyncLoader,
        const std::string& filename);

    ~SceneLoader();

    void load(
        std::shared_ptr<Registry> registry);

private:
    void attach(
        SkyboxData& skybox,
        const EntityData& root,
        const std::vector<EntityData>& entities,
        std::vector<Material>& materials,
        std::vector<Sprite>& sprited);

    void attachSkybox(
        const EntityData& root,
        SkyboxData& data,
        std::vector<Material>& materials);

    void attachVolume(
        const EntityData& root);

    void attachCubeMapCenter(
        const EntityData& root);

    void attachEntity(
        const EntityData& root,
        const EntityData& data,
        std::vector<Material>& materials,
        std::vector<Sprite>& sprites);

    MeshType* attachEntityClone(
        MeshType* type,
        const EntityData& root,
        const EntityData& entity,
        const EntityCloneData& data,
        bool cloned,
        int cloneIndex,
        std::vector<Material>& materials,
        std::vector<Sprite>& sprites);

    MeshType* attachEntityCloneRepeat(
        MeshType* type,
        const EntityData& root,
        const EntityData& entity,
        const EntityCloneData& data,
        bool cloned,
        int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& tilePositionOffset,
        std::vector<Material>& materials,
        std::vector<Sprite>& sprites);

    void assignFlags(
        const EntityCloneData& data,
        MeshType* type);

    void modifyMaterial(
        Material& m,
        const MaterialField& f,
        const Material& mod);

    MeshType* createType(
        bool isRoot,
        const EntityCloneData& data,
        const glm::uvec3& tile,
        std::vector<Material>& materials,
        std::vector<Sprite>& sprites);

    void resolveMaterial(
        MeshType* type,
        const EntityCloneData& data,
        std::vector<Material>& materials);

    void resolveSprite(
        MeshType* type,
        const EntityCloneData& data,
        std::vector<Sprite>& sprites);

    void resolveMesh(
        MeshType* type,
        const EntityCloneData& data,
        const glm::uvec3& tile);

    Node* createNode(
        MeshType* type,
        const EntityData& root,
        const EntityCloneData& data,
        const bool cloned,
        const int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& clonePositionOffset,
        const glm::vec3& tilePositionOffset,
        const bool isRoot);

    std::unique_ptr<Camera> createCamera(
        const EntityCloneData& entity,
        const CameraData& data);

    std::unique_ptr<Light> createLight(
        const EntityCloneData& entity,
        const LightData& data,
        const int cloneIndex,
        const glm::uvec3& tile);

    std::unique_ptr<CustomMaterial> createCustomMaterial(
        const EntityCloneData& entity,
        const CustomMaterialData& data,
        const int cloneIndex,
        const glm::uvec3& tile);

    std::unique_ptr<physics::Object> createPhysicsObject(
        const EntityCloneData& entity,
        const PhysicsData& data,
        const int cloneIndex,
        const glm::uvec3& tile);

    NodeController* createController(
        const EntityCloneData& entity,
        const ControllerData& data,
        Node* node);

    std::unique_ptr<NodeGenerator> createGenerator(
        const EntityCloneData& entity,
        const GeneratorData& data,
        Node* node);

    void loadMeta(
        const YAML::Node& node,
        MetaData& data);

    void loadSkybox(
        const YAML::Node& node,
        SkyboxData& data);

    void loadSkyboxFaces(
        const YAML::Node& node,
        SkyboxData& data);

    void loadRoot(
        const YAML::Node& doc,
        EntityData& root);

    void loadEntities(
        const YAML::Node& doc,
        std::vector<EntityData>& entities);

    void loadEntity(
        const YAML::Node& node,
        EntityData& data);

    void loadEntityClone(
        const YAML::Node& node,
        EntityCloneData& data,
        std::vector<EntityCloneData>& clones,
        bool recurse);

    void loadMaterialModifiers(
        const YAML::Node& node,
        EntityCloneData& data);

    void loadRepeat(
        const YAML::Node& node,
        Repeat& data);

    void loadTiling(
        const YAML::Node& node,
        Tiling& data);

    void loadCamera(
        const YAML::Node& node,
        CameraData& data);

    void loadLight(
        const YAML::Node& node,
        LightData& data);

    void loadCustomMaterial(
        const YAML::Node& node,
        CustomMaterialData& data);

    void loadPhysics(
        const YAML::Node& node,
        PhysicsData& data);

    void loadBody(
        const YAML::Node& node,
        BodyData& data);

    void loadGeom(
        const YAML::Node& node,
        GeomData& data);

    void loadController(
        const YAML::Node& node,
        ControllerData& data);

    void loadGenerator(
        const YAML::Node& node,
        GeneratorData& data);

    void loadMaterials(
        const YAML::Node& doc,
        std::vector<Material>& materials);

    void loadMaterial(
        const YAML::Node& node,
        MaterialField& fields,
        Material& material);

    void loadMaterialPbr(
        const std::string& pbrName,
        MaterialField& fields,
        Material& material);

    void loadSprites(
        const YAML::Node& doc,
        std::vector<Sprite>& sprites);

    void loadSprite(
        const YAML::Node& doc,
        Sprite& sprites);

    void loadShapes(
        const YAML::Node& doc,
        std::vector<Shape>& shapes);

    void loadShape(
        const YAML::Node& node,
        Shape& shape);

    void loadTextureSpec(
        const YAML::Node& node,
        TextureSpec& textureSpec);

    void loadTextureWrap(
        const std::string& k,
        const YAML::Node& v,
        GLint& wrapMode);

    bool readBool(const YAML::Node& node) const;
    int readInt(const YAML::Node& node) const;
    float readFloat(const YAML::Node& node) const;

    std::vector<int> readIntVector(const YAML::Node& node, int reserve) const;
    std::vector<float> readFloatVector(const YAML::Node& node, int reserve) const;

    glm::vec2 readVec2(const YAML::Node& node) const;
    glm::vec3 readVec3(const YAML::Node& node) const;
    glm::vec4 readVec4(const YAML::Node& node) const;

    glm::uvec3 readUVec3(const YAML::Node& node) const;

    glm::vec3 readScale3(const YAML::Node& node) const;
    glm::vec3 readRGB(const YAML::Node& node) const;
    glm::vec4 readRGBA(const YAML::Node& node) const;
    glm::vec2 readRefractionRatio(const YAML::Node& node) const;

    uuids::uuid resolveUUID(
        const BaseUUID& parts,
        const int cloneIndex,
        const glm::uvec3& tile);

    std::string expandMacros(
        const std::string& str,
        const int cloneIndex,
        const glm::uvec3& tile);

    BaseUUID readUUID(const YAML::Node& node);

    std::string readFile(std::string_view filename) const;

    const std::string resolveTexturePath(std::string_view line) const;

    void reportUnknown(
        std::string_view scope,
        std::string_view k,
        const YAML::Node&) const;

    std::string renderNode(
        const YAML::Node& v) const;

public:
    const std::string m_filename;
    const std::string m_dirname;

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::shared_ptr<AsyncLoader> m_asyncLoader;

    std::shared_ptr<Registry> m_registry;

    event::Dispatcher* m_dispatcher{ nullptr };

    MetaData m_meta;
    SkyboxData m_skybox;

    EntityData m_root;
    std::vector<EntityData> m_entities;
    std::vector<Material> m_materials;
    std::vector<Sprite> m_sprites;

    std::unordered_map<std::string, uuids::uuid> m_autoIds;

    Material m_defaultMaterial;
};

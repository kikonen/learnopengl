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
#include "registry/EntityType.h"

namespace event {
    class Dispatcher;
}

class Light;
class Camera;

class Registry;

class MeshType;
class Node;

class NodeGenerator;
class NodeController;

class AsyncLoader;

class SceneFile
{
    struct SkyboxData {
        std::string programName{ "skybox" };
        std::string materialName{};
        int priority{ -100 };

        bool const valid() {
            return !materialName.empty();
        }
    };

    struct MaterialField {
        bool type = false;

        bool textureSpec = false;

        bool pattern = false;
        bool reflection = false;
        bool refraction = false;
        bool refractionRatio = false;

        bool tilingX = false;
        bool tilingY = false;

        bool ns = false;

        bool ka = false;

        bool kd = false;
        bool map_kd = false;

        bool ks = false;
        bool map_ks = false;
        bool ke = false;
        bool map_ke = false;
        bool map_bump = false;
        bool ni = false;
        bool d = false;
        bool illum = false;

        bool layers = false;
        bool depth = false;

        bool map_dudv = false;
        bool map_height = false;
        bool map_noise = false;
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

        // NOTE KI debug only
        std::string targetId_str;

        uuids::uuid targetId{};

        float linear{ 0.f };
        float quadratic{ 0.f };

        float cutoffAngle{ 0.f };
        float outerCutoffAngle{ 0.f };

        glm::vec3 diffuse{ 0.5f, 0.5f, 0.5f };
        glm::vec3 specular{ 0.5f, 0.5f, 0.5f };
    };

    struct EntityCloneData {
        bool valid{ false };

        bool enabled{ false };

        EntityType type { EntityType::model };

        std::string name;
        std::string desc;

        int priority{ 0 };

        // NOTE KI debug only
        std::string id_str;
        // NOTE KI debug only
        std::string parentId_str;

        uuids::uuid id{};
        uuids::uuid parentId{};

        std::string meshName;
        std::string meshPath;

        std::string programName{};
        std::string geometryType;

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

        glm::vec3 clonePosition{ 0.f };

        bool loadTextures{ true };
        std::string materialName;
        // NOTE KI overrides *ALL* materials with defaultMaterial
        bool forceMaterial{ false };

        MaterialField materialModifierFields;
        Material materialModifiers;
        bool materialModifiers_enabled = false;

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
    SceneFile(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive,
        std::shared_ptr<AsyncLoader> asyncLoader,
        const std::string& filename);

    ~SceneFile();

    void load(
        std::shared_ptr<Registry> registry);

private:
    void attach(
        SkyboxData& skybox,
        const EntityData& root,
        const std::vector<EntityData>& entities,
        std::vector<Material>& materials);

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
        std::vector<Material>& materials);

    MeshType* attachEntityClone(
        MeshType* type,
        const EntityData& root,
        const EntityData& entity,
        const EntityCloneData& data,
        bool cloned,
        int cloneIndex,
        std::vector<Material>& materials);

    MeshType* attachEntityCloneRepeat(
        MeshType* type,
        const EntityData& root,
        const EntityData& entity,
        const EntityCloneData& data,
        bool cloned,
        int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& posAdjustment,
        std::vector<Material>& materials);

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
        std::vector<Material>& materials);

    void resolveMaterial(
        MeshType* type,
        const EntityCloneData& data,
        std::vector<Material>& materials);

    void resolveMesh(
        MeshType* type,
        const EntityCloneData& data,
        const glm::uvec3& tile);

    Node* createNode(
        MeshType* type,
        const EntityData& root,
        const EntityCloneData& data,
        bool cloned,
        int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& clonePosition,
        const glm::vec3& posAdjustment,
        bool isRoot);

    std::unique_ptr<Camera> createCamera(
        const EntityCloneData& entity,
        const CameraData& data);

    std::unique_ptr<Light> createLight(
        const EntityCloneData& entity,
        const LightData& data);

    NodeController* createController(
        const EntityCloneData& entity,
        const ControllerData& data,
        Node* node);

    std::unique_ptr<NodeGenerator> createGenerator(
        const EntityCloneData& entity,
        const GeneratorData& data,
        Node* node);

    void loadSkybox(
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

    uuids::uuid readUUID(const YAML::Node& node);

    std::string readFile(const std::string& filename) const;

    const std::string resolveTexturePath(const std::string& line) const;

    void reportUnknown(
        const std::string& scope,
        const std::string& k,
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

    SkyboxData m_skybox;

    EntityData m_root;
    std::vector<EntityData> m_entities;
    std::vector<Material> m_materials;

    std::unordered_map<std::string, uuids::uuid> m_autoIds;

    Material m_defaultMaterial;
};

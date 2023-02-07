#pragma once

#include <string>
#include <vector>
#include <map>

#include <ki/uuid.h>

#pragma warning(push)
#pragma warning(disable : 4251)
#pragma warning(disable : 4275)

#include <yaml-cpp/yaml.h>

#pragma warning(pop)

#include "asset/Material.h"
#include "registry/EntityType.h"

struct Material;
class Light;
class Camera;

class Registry;

class MeshType;
class Node;
class Group;
class NodeController;

class AsyncLoader;

class SceneFile
{
    struct SkyboxData {
        std::string shaderName{ "skybox" };
        std::string materialName{};

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

        bool fogRatio = false;

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

        bool map_dudv = false;
    };

    struct Repeat {
        int xCount{ 1 };
        int yCount{ 1 };
        int zCount{ 1 };

        double xStep{ 0 };
        double yStep{ 0 };
        double zStep{ 0 };
    };

    enum class ControllerType {
        none,
        camera,
        path,
        asteroid_belt,
        moving_light,
    };

    struct ControllerData {
        bool enabled{ false };
        ControllerType type{ ControllerType::none };

        //glm::vec3 center{ 0 };
        float speed{ 0.f };
        float radius{ 0.f };

        int mode{ 0 };
        int count{ 0 };
    };

    struct CameraData {
        bool enabled{ false };

        bool isDefault { false };

        float zoom{ 45.f };

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

        glm::vec3 pos{ 0 };

        // NOTE KI debug only
        std::string targetId_str;

        uuids::uuid targetId{};

        float linear{ 0 };
        float quadratic{ 0 };

        float cutoffAngle{ 0 };
        float outerCutoffAngle{ 0 };

        glm::vec4 ambient{ 0 };
        glm::vec4 diffuse{ 0 };
        glm::vec4 specular{ 0 };
    };

    struct EntityCloneData {
        bool valid{ false };

        bool enabled{ false };

        EntityType type { EntityType::model };

        std::string name;
        std::string desc;

        // NOTE KI debug only
        std::string id_str;
        // NOTE KI debug only
        std::string parentId_str;

        uuids::uuid id{};
        uuids::uuid parentId{};

        std::string meshName;
        std::string meshPath;

        std::string shaderName{};
        std::string geometryType;

        std::map<std::string, std::string> shaderDefinitions{};
        std::map<const std::string, bool> renderFlags{};
        glm::vec3 position{ 0 };
        glm::vec3 rotation{ 0 };
        glm::vec3 planeNormal{ 0 };
        glm::vec3 scale{ 1 };

        bool selected{ false };
        bool instanced{ false };

        glm::vec3 clonePosition{ 0 };

        bool loadTextures{ true };
        std::string materialName;
        // NOTE KI overrides *ALL* materials with defaultMaterial
        bool forceMaterial{ false };

        MaterialField materialModifierFields;
        Material materialModifiers;
        bool materialModifiers_enabled = false;

        int batchSize{ -1 };

        Repeat repeat;

        std::string initScript;
        std::string runScript;

        ControllerData controller;
        CameraData camera;
        LightData light;
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
        std::vector<Material>& materials);

    void assignFlags(
        const EntityCloneData& data,
        MeshType* type);

    void modifyMaterial(
        Material& m,
        const MaterialField& f,
        const Material& mod);

    MeshType* createType(
        const EntityData& entity,
        const EntityCloneData& data,
        std::vector<Material>& materials);

    Node* createNode(
        const Group* group,
        const EntityData& root,
        const EntityCloneData& data,
        MeshType* type,
        const glm::vec3& rootPos,
        const glm::vec3& posAdjustment,
        bool isRoot,
        bool instanced);

    std::unique_ptr<Camera> createCamera(
        const EntityCloneData& entity,
        const CameraData& data);

    std::unique_ptr<Light> createLight(
        const EntityCloneData& entity,
        const LightData& data);

    std::unique_ptr<NodeController> createController(
        const EntityCloneData& entity,
        const ControllerData& data,
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

    void loadCamera(
        const YAML::Node& node,
        CameraData& data);

    void loadLight(
        const YAML::Node& node,
        LightData& data);

    void loadController(
        const YAML::Node& node,
        ControllerData& data);

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

    glm::vec2 readVec2(const YAML::Node& node);
    glm::vec3 readVec3(const YAML::Node& node);
    glm::vec4 readVec4(const YAML::Node& node);

    glm::vec3 readScale3(const YAML::Node& node);
    glm::vec4 readRGBA(const YAML::Node& node);
    glm::vec2 readRefractionRatio(const YAML::Node& node);


    const std::string resolveTexturePath(const std::string& line);

public:
    const std::string m_filename;

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::shared_ptr<AsyncLoader> m_asyncLoader;

    std::shared_ptr<Registry> m_registry;

    SkyboxData m_skybox;

    EntityData m_root;
    std::vector<EntityData> m_entities;
    std::vector<Material> m_materials;
};

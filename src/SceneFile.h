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


#include <scene/AsyncLoader.h>


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

        bool tiling = false;

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
        float speed{ 0 };
        float radius{ 0 };

        int mode{ 0 };
        int count{ 0 };
    };

    struct CameraData {
        bool enabled{ false };

        float zoom{ 45 };

        glm::vec3 front{ 0, 0, -1 };
        glm::vec3 up{ 0, 1, 0 };

        // pos relative to owning node
        glm::vec3 pos{ 0 };
        glm::vec3 rotation{ 0 };
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

        // pos relative to owning node
        glm::vec3 pos{ 0 };
        glm::vec3 worldTarget{ 0 };

        float linear{ 0 };
        float quadratic{ 0 };

        float cutoffAngle{ 0 };
        float outerCutoffAngle{ 0 };

        glm::vec4 ambient{ 0 };
        glm::vec4 diffuse{ 0 };
        glm::vec4 specular{ 0 };
    };

    enum class EntityType {
        origo,
        model,
        quad,
        sprite,
        terrain
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

        std::string shaderName{ TEX_TEXTURE };
        std::vector<std::string> shaderDefinitions{};
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
        AsyncLoader* asyncLoader,
        const Assets& assets,
        const std::string& filename);
    ~SceneFile();

    void load(std::shared_ptr<Scene> scene);

private:
    void attach(
        std::shared_ptr<Scene> scene,
        SkyboxData& skybox,
        const EntityData& root,
        const std::vector<EntityData>& entities,
        std::vector<Material>& materials);

    void attachSkybox(
        std::shared_ptr<Scene> scene,
        SkyboxData& data,
        std::vector<Material>& materials);

    void attachEntity(
        std::shared_ptr<Scene> scene,
        const EntityData& root,
        const EntityData& data,
        std::vector<Material>& materials);

    std::shared_ptr<NodeType> attachEntityClone(
        std::shared_ptr<Scene> scene,
        std::shared_ptr<NodeType>& type,
        const EntityData& root,
        const EntityData& entity,
        const EntityCloneData& data,
        bool cloned,
        std::vector<Material>& materials);

    void assignFlags(
        const EntityCloneData& data,
        NodeType& type);

    void modifyMaterial(
        Material& m,
        const MaterialField& f,
        const Material& mod);

    std::shared_ptr<NodeType> createType(
        const EntityData& entity,
        const EntityCloneData& data,
        std::vector<Material>& materials);

    Node* createNode(
        const Group* group,
        const EntityData& root,
        const EntityCloneData& data,
        const std::shared_ptr<NodeType>& type,
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

private:
    AsyncLoader* m_asyncLoader;
    const Assets& m_assets;
    const std::string m_filename;

    SkyboxData m_skybox;

    EntityData m_root;
    std::vector<EntityData> m_entities;
    std::vector<Material> m_materials;
};

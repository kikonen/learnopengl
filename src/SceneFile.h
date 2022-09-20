#pragma once

#include <string>
#include <map>

#include <stduuid/uuid.h>

#pragma warning(push)
#pragma warning(disable : 4251)
#pragma warning(disable : 4275)

#include <yaml-cpp/yaml.h>

#pragma warning(pop)


#include <scene/AsyncLoader.h>


class SceneFile
{
    enum class EntityType {
        node,
        sprite
    };

    struct SkyboxData {
        std::string shaderName{ "skybox" };
        std::string materialName{};

        bool valid() {
            return !materialName.empty();
        }
    };

    struct MaterialField {
        bool reflection = false;
        bool refraction = false;
        bool refractionRatio = false;
        bool textureSpec = false;
        bool tiling = false;

        bool any() {
            return reflection || refraction || refractionRatio || textureSpec || tiling;
        }
    };

    struct Repeat {
        int xCount{ 1 };
        int yCount{ 1 };
        int zCount{ 1 };

        double xStep{ 0 };
        double yStep{ 0 };
        double zStep{ 0 };
    };

    struct EntityData {
        int typeId{ 0 };
        bool enabled{ false };
        bool valid{ false };

        EntityType type { EntityType::node };

        std::string name;
        std::string desc;

        uuids::uuid id{};
        uuids::uuid parentId{};

        std::string modelName;
        std::string modelPath;
        std::string shaderName{ TEX_TEXTURE };
        std::vector<std::string> shaderDefinitions{};
        std::map<const std::string, bool> renderFlags{};
        //glm::vec3 pos{ 0 };
        std::vector<glm::vec3> positions{};
        glm::vec3 rotation{ 0 };
        glm::vec4 mirrorPlane{ 0 };
        double scale{ 1 };

        bool selected{ false };

        bool loadTextures{ true };
        std::shared_ptr<Material> defaultMaterial;
        // NOTE KI overrides *ALL* materials with defaultMaterial
        bool overrideMaterials{ false };

        MaterialField materialModifierFields;
        std::shared_ptr<Material> materialModifiers;

        int batchSize{ -1 };

        Repeat repeat;
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
        std::map<const uuids::uuid, EntityData>& entities,
        std::vector<std::shared_ptr<Material>>& materials);

    void attachSkybox(
        std::shared_ptr<Scene> scene,
        SkyboxData& data,
        std::vector<std::shared_ptr<Material>>& materials);

    void attachEntity(
        std::shared_ptr<Scene> scene,
        const EntityData& data,
        std::map<const uuids::uuid, EntityData>& entities,
        std::vector<std::shared_ptr<Material>>& materials);

    void loadSkybox(
        const YAML::Node& node,
        SkyboxData& data,
        std::vector<std::shared_ptr<Material>>& materials);

    void loadEntities(
        const YAML::Node& doc,
        std::map<const uuids::uuid, EntityData>& entities,
        std::vector<std::shared_ptr<Material>>& materials);

    void loadEntity(
        const YAML::Node& node,
        std::vector<std::shared_ptr<Material>>& materials,
        EntityData& data);

    void loadMaterialModifiers(
        const YAML::Node& node,
        EntityData& data);

    void loadRepeat(
        const YAML::Node& node,
        EntityData& data);

    void loadMaterials(
        const YAML::Node& doc,
        std::vector<std::shared_ptr<Material>>& materials);

    void loadMaterial(
        const YAML::Node& node,
        MaterialField& fields,
        std::shared_ptr<Material>& material);

    void loadTextureSpec(
        const YAML::Node& node,
        TextureSpec& textureSpec);

    glm::vec2 readVec2(const YAML::Node& node);
    glm::vec3 readVec3(const YAML::Node& node);
    glm::vec4 readVec4(const YAML::Node& node);

    glm::vec4 readRGBA(const YAML::Node& node);
    glm::vec2 readRefractionRatio(const YAML::Node& node);


    const std::string resolveTexturePath(const std::string& line);

private:
    AsyncLoader* asyncLoader;
    const Assets& assets;
    const std::string filename;

    SkyboxData skybox;
    std::map<const uuids::uuid, EntityData> entities;
    std::vector<std::shared_ptr<Material>> materials;
};

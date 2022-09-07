#pragma once

#include <string>
#include <map>

#include <yaml-cpp/yaml.h>

#include <scene/AsyncLoader.h>


class SceneFile
{
	struct MaterialField {
		bool reflection = false;
		bool refraction = false;
		bool refractionRatio = false;

        bool any() {
            return reflection || refraction || refractionRatio;
        }
	};

	struct EntityData {
		int typeId{ 0 };
		std::string name{};
		std::string modelName{};
		std::string modelPath{ "/" };
		std::string shaderName{ TEX_TEXTURE };
		std::vector<std::string> shaderDefinitions{};
		std::map<const std::string, bool> renderFlags{};
		glm::vec3 pos{ 0 };
		glm::vec3 rotation{ 0 };
		glm::vec4 mirrorPlane{ 0 };
		double scale{ 1 };

		std::shared_ptr<Material> defaultMaterial;
		// NOTE KI overrides *ALL* materials with defaultMaterial
		bool overrideMaterials{ false };

		MaterialField materialModifierFields;
		std::shared_ptr<Material> materialModifiers;
	};

public:
	SceneFile(
		const Assets& assets,
		const std::string& filename);
	~SceneFile();

	std::shared_ptr<Scene> load(std::shared_ptr<Scene> scene);

private:
	void testYAML();

	void loadEntities(
		const YAML::Node& doc,
		std::map<const std::string, std::shared_ptr<Material>>& materials);

	void loadEntity(
		const YAML::Node& node,
		std::map<const std::string, std::shared_ptr<Material>>& materials);

	void loadMaterialModifiers(
		const YAML::Node& node,
		EntityData& data);

	void loadMaterials(
		const YAML::Node& doc,
		std::map<const std::string, std::shared_ptr<Material>>& materials);

	void loadMaterial(
		const YAML::Node& node,
		MaterialField& fields,
		std::shared_ptr<Material>& material);

	glm::vec3 readVec3(const YAML::Node& node);
	glm::vec4 readVec4(const YAML::Node& node);
	double readRefractionRatio(const YAML::Node& node);

	const std::string resolveTexturePath(const std::string& line);

private:
	AsyncLoader loader;
	const Assets& assets;
	const std::string filename;
};


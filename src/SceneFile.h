#pragma once

#include <string>
#include <map>

#include <yaml-cpp/yaml.h>

#include <scene/AsyncLoader.h>

class SceneFile
{
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

	void loadMaterials(
		const YAML::Node& doc,
		std::map<const std::string, std::shared_ptr<Material>>& materials);

	const glm::vec3 readVec3(const YAML::Node& node);
	const glm::vec4 readVec4(const YAML::Node& node);
	const std::string resolveTexturePath(const std::string& line);

private:
	AsyncLoader loader;
	const Assets& assets;
	const std::string filename;
};


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

	Scene* load(Scene* scene);

private:
	void testYAML();

	void loadMaterials(const YAML::Node& doc);
	void loadEntities(const YAML::Node& doc);

	const glm::vec3 readVec3(const YAML::Node& node);
	const glm::vec4 readVec4(const YAML::Node& node);
	const std::string resolveTexturePath(const std::string& line);

private:
	AsyncLoader loader;
	const Assets& assets;
	const std::string filename;
	std::map<std::string, Material*> materials;
};


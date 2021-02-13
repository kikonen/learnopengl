#include "SceneLoader.h"


SceneLoader::SceneLoader(const Assets& assets)
	: assets(assets)
{
}

void SceneLoader::setup()
{
}

void SceneLoader::addLoader(std::function<void()> loader)
{
	loaders.push_back(loader);
}

void SceneLoader::load()
{
	for (auto& loader : loaders) {
		startedLoaders.push_back(std::async(std::launch::async, loader));
	}
}

Shader* SceneLoader::getShader(const std::string& name, const std::string& geometryType)
{
	return Shader::getShader(assets, name, geometryType);
}

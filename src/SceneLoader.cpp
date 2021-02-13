#include "SceneLoader.h"


SceneLoader::SceneLoader(const Assets& assets)
	: assets(assets)
{
}

void SceneLoader::setup()
{
}

std::future<void>* SceneLoader::addLoader(std::function<void()> loader)
{
	startedLoaders.push_back(std::async(std::launch::async, loader));
	std::future<void>* f = &startedLoaders[startedLoaders.size() - 1];
	return f;
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

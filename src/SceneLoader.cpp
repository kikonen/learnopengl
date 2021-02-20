#include "SceneLoader.h"


SceneLoader::SceneLoader(const Assets& assets)
	: assets(assets)
{
}

void SceneLoader::setup()
{
}

int SceneLoader::addLoader(std::function<void()> loader)
{
	std::lock_guard<std::mutex> lock(load_lock);
	loaders.emplace_back(std::async(std::launch::async, loader));
	int index = loaders.size() - 1;
	return index;
}

const std::future<void>& SceneLoader::getLoader(int index)
{
	return loaders[index];
}

void SceneLoader::load()
{
	//for (auto& loader : loaders) {
	//	startedLoaders.push_back(std::async(std::launch::async, loader));
	//}
}


Shader* SceneLoader::getShader(const std::string& name, const std::string& geometryType)
{
	return Shader::getShader(assets, name, geometryType);
}

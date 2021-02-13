#include "SceneSetup.h"


SceneSetup::SceneSetup(const Assets& assets)
	: assets(assets)
{
}

void SceneSetup::addLoader(std::function<void()> loader)
{
	loaders.push_back(loader);
}

void SceneSetup::load()
{
	for (auto& loader : loaders) {
		startedLoaders.push_back(std::async(std::launch::async, loader));
	}
}

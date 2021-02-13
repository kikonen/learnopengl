#pragma once

#include <future>

#include "Assets.h"
#include "Scene.h"

class SceneSetup
{
public:
	SceneSetup(const Assets& assets);

	void addLoader(std::function<void()> loader);
	void load();

public:
	const Assets& assets;
	Scene* scene = nullptr;

protected:
	std::vector<std::function<void()>> loaders;

	// https://stackoverflow.com/questions/20126551/storing-a-future-in-a-list
	std::vector<std::future<void>> startedLoaders;
};


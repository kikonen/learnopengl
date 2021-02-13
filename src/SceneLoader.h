#pragma once

#include <future>

#include "Assets.h"
#include "Scene.h"

class SceneLoader
{
public:
	SceneLoader(const Assets& assets);

	virtual void setup();

	void addLoader(std::function<void()> loader);
	void load();

protected:
	Shader* getShader(const std::string& name, const std::string& geometryType = "");

public:
	const Assets& assets;
	Scene* scene = nullptr;

protected:
	std::vector<std::function<void()>> loaders;

	// https://stackoverflow.com/questions/20126551/storing-a-future-in-a-list
	std::vector<std::future<void>> startedLoaders;
};


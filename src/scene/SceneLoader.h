#pragma once

#include <future>
#include <mutex>

#include "asset/Assets.h"
#include "Scene.h"

class SceneLoader
{
public:
	SceneLoader(const Assets& assets);

	virtual void setup();

	size_t addLoader(std::function<void()> loader);
	void load();
	const std::future<void>& getLoader(unsigned int index);

protected:
	Shader* getShader(
		const std::string& name);

	Shader* getShader(
		const std::string& name,
		const std::vector<std::string>& defines);

public:
	const Assets& assets;
	Scene* scene = nullptr;

protected:
	// https://stackoverflow.com/questions/20126551/storing-a-future-in-a-list
	std::vector<std::future<void>> loaders;

	std::mutex load_lock;
};


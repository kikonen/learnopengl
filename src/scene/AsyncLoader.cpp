#include "AsyncLoader.h"


AsyncLoader::AsyncLoader(const Assets& assets)
	: assets(assets)
{
}

void AsyncLoader::setup()
{
}

size_t AsyncLoader::addLoader(std::function<void()> loader)
{
	std::lock_guard<std::mutex> lock(load_lock);
	loaders.emplace_back(std::async(std::launch::async, loader));
	return loaders.size() - 1;
}

const std::future<void>& AsyncLoader::getLoader(unsigned int index)
{
	return loaders[index];
}

std::shared_ptr<Shader> AsyncLoader::getShader(const std::string& name)
{
	return Shader::getShader(assets, name);
}

std::shared_ptr<Shader> AsyncLoader::getShader(const std::string& name, const std::vector<std::string>& defines)
{
	return Shader::getShader(assets, name, "", defines);
}

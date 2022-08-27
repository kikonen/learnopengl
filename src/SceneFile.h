#pragma once

#include <iostream>

#include <scene/AsyncLoader.h>

class SceneFile
{
public:
	SceneFile(
		const Assets& assets,
		const std::string& filename);
	~SceneFile();

	Scene* load();

private:
	void testYAML();

private:
	const Assets& assets;
	const std::string filename;
};


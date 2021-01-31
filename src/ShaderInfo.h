#pragma once

#include "Shader.h"

class ShaderInfo
{
public:
	ShaderInfo(Shader* shader);

	int prepare();
	void bind();
	void unbind();
public:
	Shader* shader;

	bool preparedNode = false;
};


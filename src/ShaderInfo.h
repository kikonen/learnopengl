#pragma once

#include "Shader.h"

class ShaderInfo
{
public:
	ShaderInfo(Shader* shader);

	int prepare();
	void bind();
public:
	unsigned int VBO = 0;
	unsigned int VAO = 0;
	unsigned int EBO = 0;

	Shader* shader;
	const bool bindTexture;
};


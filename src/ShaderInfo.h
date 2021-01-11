#pragma once

#include "Shader.h"

class ShaderInfo
{
public:
	ShaderInfo(Shader* shader, bool stencil, bool useTexture);

	int prepare();
	void bind();
public:
	unsigned int VBO = 0;
	unsigned int VAO = 0;
	unsigned int EBO = 0;

	const bool stencil;
	const bool useTexture;
	Shader* shader;
};


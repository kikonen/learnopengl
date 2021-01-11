#include "ShaderInfo.h"

ShaderInfo::ShaderInfo(Shader* shader, bool stencil, bool useTexture)
	: shader(shader),
	stencil(stencil),
	useTexture(useTexture)
{
}

int ShaderInfo::prepare()
{
	if (shader->setup()) {
		return -1;
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	return 0;
}

void ShaderInfo::bind()
{
	shader->use();
}


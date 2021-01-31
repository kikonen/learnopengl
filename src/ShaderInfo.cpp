#include "ShaderInfo.h"

ShaderInfo::ShaderInfo(Shader* shader)
	: shader(shader)
{
}

int ShaderInfo::prepare()
{
	if (shader->setup()) {
		return -1;
	}

	return 0;
}

void ShaderInfo::bind()
{
	shader->bind();
}

void ShaderInfo::unbind()
{
	shader->unbind();
}


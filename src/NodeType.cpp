#include "NodeType.h"

#include "Assets.h"
#include "Mesh.h"
#include "RenderContext.h"

int objectIDbase = 0;

int NodeType::nextID()
{
	return ++objectIDbase;
}

NodeType::NodeType(int objectID, Shader* defaultShader)
	: objectID(objectID),
	defaultShader(defaultShader)
{
}

NodeType::~NodeType()
{
}

void NodeType::prepare(const Assets& assets)
{
	if (!mesh) return;
	mesh->prepare(assets);
}

Shader* NodeType::bind(const RenderContext& ctx, Shader* shader)
{
	if (!mesh) return nullptr;

	shader = shader ? shader : defaultShader;
	if (!shader) return nullptr;
	boundShader = shader;

	shader->bind();
	mesh->bind(ctx, shader);
	ctx.bind(shader);

	if (renderBack) {
		glDisable(GL_CULL_FACE);
	}
	else {
		glEnable(GL_CULL_FACE);
	}

	return shader;
}

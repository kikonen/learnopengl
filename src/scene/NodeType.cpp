#include "NodeType.h"

#include "asset/Assets.h"
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

	if (defaultShader) {
		defaultShader->prepare();
		defaultShader->reflectionMap.set(assets.reflectionMapUnitIndex);
		defaultShader->refractionMap.set(assets.refractionMapUnitIndex);
		defaultShader->shadowMap.set(assets.shadowMapUnitIndex);
		defaultShader->skybox.set(assets.skyboxUnitIndex);
	}

	if (batchMode && batch.size > 0) {
		batch.prepare(this);
	}
	else {
		batch.size = 0;
	}
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

	shader->drawInstanced.set(false);
	shader->hasReflectionMap.set(reflection);

	if (renderBack) {
		glDisable(GL_CULL_FACE);
	}
	else {
		glEnable(GL_CULL_FACE);
	}

	return shader;
}

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
	KI_GL_CALL(mesh->prepare(assets));

	if (defaultShader) {
		KI_GL_CALL(defaultShader->prepare());
	}
	
	if (batchMode && batch.size > 0) {
		KI_GL_CALL(batch.prepare(this));
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

	//shader->drawInstanced.set(false);
	shader->hasReflectionMap.set(reflection);

	KI_GL_CALL(shader->reflectionMap.set(ctx.assets.reflectionMapUnitIndex));
	KI_GL_CALL(shader->refractionMap.set(ctx.assets.refractionMapUnitIndex));
	KI_GL_CALL(shader->shadowMap.set(ctx.assets.shadowMapUnitIndex));
	KI_GL_CALL(shader->skybox.set(ctx.assets.skyboxUnitIndex));

	if (renderBack) {
		glDisable(GL_CULL_FACE);
	}
	else {
		glEnable(GL_CULL_FACE);
	}

	return shader;
}

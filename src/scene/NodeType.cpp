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

bool NodeType::hasReflection()
{
	if (!mesh) return false;
	return mesh->hasReflection();
}

bool NodeType::hasRefraction()
{
	if (!mesh) return false;
	return mesh->hasRefraction();
}

void NodeType::setReflection(float reflection)
{
	mesh->setReflection(reflection);
}

void NodeType::setRefraction(float refraction)
{
	mesh->setRefraction(refraction);
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

	shader->noiseTex.set(ctx.assets.noiseUnitIndex);
	shader->reflectionTex.set(ctx.assets.waterReflectionMapUnitIndex);
	shader->refractionTex.set(ctx.assets.waterRefractionMapUnitIndex);

	shader->reflectionMap.set(ctx.assets.reflectionMapUnitIndex);
	shader->refractionMap.set(ctx.assets.refractionMapUnitIndex);
	shader->shadowMap.set(ctx.assets.shadowMapUnitIndex);
	shader->skybox.set(ctx.assets.skyboxUnitIndex);

	if (renderBack) {
		ctx.state.disable(GL_CULL_FACE);
	}
	else {
		ctx.state.enable(GL_CULL_FACE);
	}

	return shader;
}

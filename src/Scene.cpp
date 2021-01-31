#include "Scene.h"

#include "KIGL.h"


Scene::Scene(const Assets& assets)
	: assets(assets)
{
	nodeRenderer = new NodeRenderer(assets);
	terrainRenderer = new TerrainRenderer(assets);
	shadowMapRenderer = new ShadowMapRenderer(assets);
	normalRenderer = new NormalRenderer(assets);
}

Scene::~Scene()
{
	delete shadowMapRenderer;
	delete normalRenderer;
}

void Scene::prepare()
{
	for (auto node : nodes) {
		node->prepare();
	}

	// NOTE KI OpenGL does NOT like interleaved draw and prepare
	nodeRenderer->prepare(nodes);

	if (showNormals) {
		normalRenderer->prepare(nodes);
	}

	shadowMapRenderer->prepare();
}

void Scene::bind(RenderContext& ctx)
{
	shadowMapRenderer->bind(ctx);
	ctx.bindGlobal();
}

void Scene::draw(RenderContext& ctx)
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// https://cmichel.io/understanding-front-faces-winding-order-and-normals
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_DEPTH_TEST);

	shadowMapRenderer->draw(ctx, nodes);

	if (dirLight) {
		dirLight->pos = shadowMapRenderer->lightPos;
		dirLight->dir = shadowMapRenderer->lightDir;
	}

	glActiveTexture(ctx.engine.assets.shadowMapUnitId);
	glBindTexture(GL_TEXTURE_2D, shadowMapRenderer->shadowMap);

	if (skyboxRenderer) {
		skyboxRenderer->draw(ctx);
	}

	nodeRenderer->render(ctx, nodes);

	if (showNormals) {
		normalRenderer->render(ctx, nodes);
	}

	shadowMapRenderer->drawDebug(ctx);

	KIGL::checkErrors("scene.draw");
}


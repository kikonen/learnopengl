#include "Scene.h"

#include "KIGL.h"


Scene::Scene(const Assets& assets)
	: assets(assets)
{
	nodeRenderer = new NodeRenderer(assets);
	spriteRenderer = new SpriteRenderer(assets);
	terrainRenderer = new TerrainRenderer(assets);
	viewportRenderer = new ViewportRenderer(assets);

	shadowMapRenderer = new ShadowMapRenderer(assets);
	normalRenderer = new NormalRenderer(assets);
}

Scene::~Scene()
{
	delete nodeRenderer;
	delete spriteRenderer;
	delete terrainRenderer;
	delete viewportRenderer;

	delete shadowMapRenderer;
	delete normalRenderer;
}

void Scene::prepare()
{
	for (auto node : nodes) {
		node->prepare(assets);
	}

	for (auto sprite : sprites) {
		sprite->prepare(assets);
	}

	for (auto terrain : terrains) {
		terrain->prepare(assets);
	}

	// NOTE KI OpenGL does NOT like interleaved draw and prepare
	nodeRenderer->prepare();
	spriteRenderer->prepare();

	terrainRenderer->prepare();
	viewportRenderer->prepare();

	if (showNormals) {
		normalRenderer->prepare();
	}

	shadowMapRenderer->prepare();

	viewports.push_back(shadowMapRenderer->debugViewport);
}

void Scene::update(RenderContext& ctx)
{
	if (skyboxRenderer) {
		skyboxRenderer->update(ctx);
	}

	nodeRenderer->update(ctx, nodes);

	spriteRenderer->update(ctx, sprites);

	if (showNormals) {
		normalRenderer->update(ctx, nodes);

		std::vector<Node*> r(terrains.begin(), terrains.end());
		normalRenderer->update(ctx, r);
	}

	terrainRenderer->update(ctx, terrains);
	viewportRenderer->update(ctx, viewports);
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

	shadowMapRenderer->render(ctx, nodes);

	shadowMapRenderer->bindTexture(ctx);

	if (skyboxRenderer) {
		skyboxRenderer->render(ctx);
	}

	nodeRenderer->render(ctx, nodes);

	spriteRenderer->render(ctx, sprites);

	if (showNormals) {
		normalRenderer->render(ctx, nodes);

		std::vector<Node*> r(terrains.begin(), terrains.end());
		normalRenderer->render(ctx, r);
	}

	terrainRenderer->render(ctx, terrains);
	
	viewportRenderer->render(ctx, viewports);

	KIGL::checkErrors("scene.draw");
}


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
	for (auto& x : typeNodes) {
		for (auto& e : x.second) {
			e->prepare(assets);
		}
	}

	for (auto& x : typeSprites) {
		for (auto& e : x.second) {
			e->prepare(assets);
		}
	}

	for (auto& x : typeTerrains) {
		for (auto& e : x.second) {
			e->prepare(assets);
		}
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
	if (dirLight) {
		dirLight->update(ctx);
	}
	for (auto light : pointLights) {
		light->update(ctx);
	}
	for (auto light : spotLights) {
		light->update(ctx);
	}

	if (skyboxRenderer) {
		skyboxRenderer->update(ctx);
	}

	nodeRenderer->update(ctx, typeNodes);
	spriteRenderer->update(ctx, typeSprites);
	terrainRenderer->update(ctx, typeTerrains);
	viewportRenderer->update(ctx, viewports);

	if (showNormals) {
		normalRenderer->update(ctx, typeNodes);

		std::map<int, std::vector<Node*>> r1 = spriteToNodes();
		normalRenderer->update(ctx, r1);

		std::map<int, std::vector<Node*>> r2 = terrainToNodes();
		normalRenderer->update(ctx, r1);
	}
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

	shadowMapRenderer->render(ctx, typeNodes, typeSprites, typeTerrains);

	shadowMapRenderer->bindTexture(ctx);

	if (skyboxRenderer) {
		skyboxRenderer->render(ctx);
	}

	terrainRenderer->render(ctx, typeTerrains);
	spriteRenderer->render(ctx, typeSprites);
	nodeRenderer->render(ctx, typeNodes);

	if (showNormals) {
		normalRenderer->render(ctx, typeNodes);

		std::map<int, std::vector<Node*>> r1 = spriteToNodes();
		normalRenderer->render(ctx, r1);

		std::map<int, std::vector<Node*>> r2 = terrainToNodes();
		normalRenderer->render(ctx, r1);
	}

	viewportRenderer->render(ctx, viewports);

	KIGL::checkErrors("scene.draw");
}

Light* Scene::getDirLight()
{
	return dirLight;
}

std::vector<Light*>& Scene::getPointLights()
{
	return pointLights;
}

std::vector<Light*>& Scene::getSpotLights()
{
	return spotLights;
}

void Scene::addLight(Light* light)
{
	if (light->directional) {
		dirLight = light;
	}
	else if (light->point) {
		pointLights.push_back(light);
	}
	else if (light->spot) {
		spotLights.push_back(light);
	}
}

void Scene::addNode(Node* node)
{
	typeNodes[node->objectID].push_back(node);
}

void Scene::addSprite(Sprite* sprite)
{
	typeSprites[sprite->objectID].push_back(sprite);
}

void Scene::addTerrain(Terrain* terrain)
{
	typeTerrains[terrain->objectID].push_back(terrain);
}

void Scene::addViewPort(Viewport* viewport)
{
	viewports.push_back(viewport);
}

std::map<int, std::vector<Node*>> Scene::terrainToNodes()
{
	std::map<int, std::vector<Node*>> r;

	for (auto& x : typeTerrains) {
		auto a = r[x.first];
		a.reserve(x.second.size());
		for (auto& e : x.second) {
			a.push_back(e);
		}
	}
	return r;
}

std::map<int, std::vector<Node*>> Scene::spriteToNodes()
{
	std::map<int, std::vector<Node*>> r;

	for (auto& x : typeSprites) {
		auto a = r[x.first];
		a.reserve(x.second.size());
		for (auto& e : x.second) {
			a.push_back(e);
		}
	}
	return r;
}


#include "Scene.h"

#include <thread>
#include <mutex>

#include "ki/GL.h"
#include "ki/Timer.h"

#include "asset/UBO.h"


Scene::Scene(const Assets& assets)
	: assets(assets),
	registry(*this)
{
	nodeRenderer = new NodeRenderer(assets);
	spriteRenderer = new SpriteRenderer(assets);
	terrainRenderer = new TerrainRenderer(assets);
	viewportRenderer = new ViewportRenderer(assets);

	shadowMapRenderer = new ShadowMapRenderer(assets);
	reflectionMapRenderer = new ReflectionMapRenderer(assets);
	normalRenderer = new NormalRenderer(assets);

	particleSystem = new ParticleSystem(assets);
}

Scene::~Scene()
{
	delete nodeRenderer;
	delete spriteRenderer;
	delete terrainRenderer;
	delete viewportRenderer;

	delete shadowMapRenderer;
	delete reflectionMapRenderer;
	delete normalRenderer;
}

void Scene::prepare()
{
	prepareUBOs();

	// NOTE KI OpenGL does NOT like interleaved draw and prepare
	nodeRenderer->prepare();
	spriteRenderer->prepare();

	terrainRenderer->prepare();
	viewportRenderer->prepare();

	shadowMapRenderer->prepare();
	reflectionMapRenderer->prepare();

	if (showNormals) {
		normalRenderer->prepare();
	}

	particleSystem->prepare();

	registry.addViewPort(shadowMapRenderer->debugViewport);
}


void Scene::processEvents(RenderContext& ctx) 
{
}

void Scene::update(RenderContext& ctx)
{
	KI_GL_CALL(registry.attachNodes());

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
		KI_GL_CALL(skyboxRenderer->update(ctx, registry));
	}

	KI_GL_CALL(nodeRenderer->update(ctx, registry));
	KI_GL_CALL(spriteRenderer->update(ctx, registry));
	KI_GL_CALL(terrainRenderer->update(ctx, registry));
	KI_GL_CALL(viewportRenderer->update(ctx, registry));

	KI_GL_CALL(particleSystem->update(ctx));
}

void Scene::bind(RenderContext& ctx)
{
	shadowMapRenderer->bind(ctx);
	reflectionMapRenderer->bind(ctx);
	ctx.bindUBOs();

	if (!mirrorBuffer && showMirrorView) {
		mirrorBuffer = new TextureBuffer(640, 480);
		mirrorBuffer->prepare();

		mirrorViewport = new Viewport(
			glm::vec3(0.5, 1, 0),
			glm::vec3(0, 0, 0),
			glm::vec2(0.5f, 0.5f),
			mirrorBuffer->textureID,
			Shader::getShader(assets, TEX_VIEWPORT));

		mirrorViewport->prepare();
		registry.addViewPort(mirrorViewport);
	}
}

void Scene::draw(RenderContext& ctx)
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.9f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// https://cmichel.io/understanding-front-faces-winding-order-and-normals
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_DEPTH_TEST);

	shadowMapRenderer->render(ctx, registry);
	shadowMapRenderer->bindTexture(ctx);

	reflectionMapRenderer->render(ctx, registry, skyboxRenderer);

	// "back mirror" viewport
	if (showMirrorView) {
		mirrorBuffer->bind();
		glViewport(0, 0, mirrorBuffer->width, mirrorBuffer->height);

		glClearColor(0.9f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		Camera camera(ctx.camera->getPos(), ctx.camera->getFront(), ctx.camera->getUp());
		camera.setZoom(ctx.camera->getZoom());
		camera.setRotation(ctx.camera->getRotation() + glm::vec3(0, 180, 0));
		RenderContext mirrorCtx(ctx.engine, ctx.dt, ctx.scene, &camera, mirrorBuffer->width, mirrorBuffer->height);
		mirrorCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;
		mirrorCtx.bindUBOs();

		if (skyboxRenderer) {
			skyboxRenderer->render(mirrorCtx, registry);
		}

		reflectionMapRenderer->bindTexture(mirrorCtx);

		terrainRenderer->render(mirrorCtx, registry);
		spriteRenderer->render(mirrorCtx, registry);
		nodeRenderer->render(mirrorCtx, registry);

		particleSystem->render(mirrorCtx);

		if (showNormals) {
			normalRenderer->render(mirrorCtx, registry);
		}

		mirrorBuffer->unbind();
		glViewport(0, 0, ctx.width, ctx.height);

		ctx.bindUBOs();
	}

	{
		reflectionMapRenderer->bindTexture(ctx);

		if (skyboxRenderer) {
			skyboxRenderer->render(ctx, registry);
		}


		terrainRenderer->render(ctx, registry);
		spriteRenderer->render(ctx, registry);
		nodeRenderer->render(ctx, registry);

		particleSystem->render(ctx);

		if (showNormals) {
			normalRenderer->render(ctx, registry);
		}

		glDisable(GL_DEPTH_TEST);
		viewportRenderer->render(ctx, registry);
	}

	//KI_GL_DEBUG("scene.draw");
}

Camera* Scene::getCamera()
{
	return cameraNode ? cameraNode->camera : nullptr;
}

Node* Scene::getCameraNode()
{
	return cameraNode;
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

void Scene::addCamera(Node* node)
{
	if (!node->camera) return;
	cameraNode = node;
}

void Scene::addLight(Node* node)
{
	Light* light = node->light;
	if (!light) return;

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

void Scene::prepareUBOs()
{
	// Matrices
	{
		glGenBuffers(1, &ubo.matrices);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo.matrices);
		// projection + view
		int sz = sizeof(MatricesUBO);
		glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATRICES, ubo.matrices, 0, sz);
		ubo.matricesSize = sz;
	}
	// Data
	{
		glGenBuffers(1, &ubo.data);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo.data);

		// cameraPos + time
		int sz = sizeof(DataUBO);
		glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_DATA, ubo.data, 0, sz);
		ubo.dataSize = sz;
	}
	// Lights
	{
		glGenBuffers(1, &ubo.lights);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo.lights);
		// DirLight + PointLight + SpotLight
		int sz = sizeof(LightsUBO);
		int sz2 = sizeof(DirLightUBO);
		int sz3 = sizeof(PointLightUBO);
		int sz4 = sizeof(SpotLightUBO);
		glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_LIGHTS, ubo.lights, 0, sz);
		ubo.lightsSize = sz;
	}

	// materials
	{
		glGenBuffers(1, &ubo.materials);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo.materials);
		int sz = sizeof(MaterialsUBO);
		int sz2 = sizeof(MaterialUBO);
		glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_STREAM_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATERIALS, ubo.materials, 0, sz);
		ubo.materialsSize = sz;
	}
}

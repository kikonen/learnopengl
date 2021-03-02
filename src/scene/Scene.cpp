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
	//terrainRenderer = new TerrainRenderer(assets);

	viewportRenderer = new ViewportRenderer(assets);

	waterMapRenderer = new WaterMapRenderer(assets);
	reflectionMapRenderer = new ReflectionMapRenderer(assets);
	shadowMapRenderer = new ShadowMapRenderer(assets);

	normalRenderer = new NormalRenderer(assets);

	particleSystem = new ParticleSystem(assets);
}

Scene::~Scene()
{
	delete nodeRenderer;
	//delete terrainRenderer;

	delete viewportRenderer;

	delete waterMapRenderer;
	delete reflectionMapRenderer;
	delete shadowMapRenderer;

	delete normalRenderer;

	delete particleSystem;
}

void Scene::prepare()
{
	prepareUBOs();

	// NOTE KI OpenGL does NOT like interleaved draw and prepare
	nodeRenderer->prepare();
	//terrainRenderer->prepare();

	viewportRenderer->prepare();

	waterMapRenderer->prepare();
	reflectionMapRenderer->prepare();
	shadowMapRenderer->prepare();

	if (showNormals) {
		normalRenderer->prepare();
	}

	particleSystem->prepare();

	{
		mainViewport = new Viewport(
			glm::vec3(-0.75, 0.75, 0),
			glm::vec3(0, 0, 0),
			glm::vec2(1.5f, 1.5f),
			-1,
			Shader::getShader(assets, TEX_VIEWPORT));
		
		mainViewport->effect = ViewportEffect::grayScale;

		mainViewport->prepare();
		registry.addViewPort(mainViewport);
	}

	if (!mirrorBuffer && showMirrorView) {
		mirrorBuffer = new TextureBuffer({ 640, 480 });
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

	//registry.addViewPort(shadowMapRenderer->debugViewport);
	//registry.addViewPort(waterMapRenderer->reflectionDebugViewport);
	//registry.addViewPort(waterMapRenderer->refractionDebugViewport);
}


void Scene::processEvents(RenderContext& ctx) 
{
}

void Scene::update(RenderContext& ctx)
{
	registry.attachNodes();

	if (dirLight) {
		dirLight->update(ctx);
	}
	for (auto light : pointLights) {
		light->update(ctx);
	}
	for (auto light : spotLights) {
		light->update(ctx);
	}

	for (auto generator : particleGenerators) {
		generator->update(ctx);
	}

	if (skyboxRenderer) {
		skyboxRenderer->update(ctx, registry);
	}

	nodeRenderer->update(ctx, registry);
	//terrainRenderer->update(ctx, registry);

	viewportRenderer->update(ctx, registry);

	particleSystem->update(ctx);

	updateMainViewport(ctx);
}

void Scene::bind(RenderContext& ctx)
{
	nodeRenderer->bind(ctx);
	//terrainRenderer->bind(ctx);

	viewportRenderer->bind(ctx);

	waterMapRenderer->bind(ctx);
	reflectionMapRenderer->bind(ctx);
	shadowMapRenderer->bind(ctx);

	ctx.bindGlobal();
	ctx.bindUBOs();
}

void Scene::draw(RenderContext& ctx)
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// https://cmichel.io/understanding-front-faces-winding-order-and-normals
	ctx.state.enable(GL_CULL_FACE);
	ctx.state.cullFace(GL_BACK); 
	ctx.state.frontFace(GL_CCW); 

	ctx.state.enable(GL_DEPTH_TEST);

	shadowMapRenderer->render(ctx, registry);
	shadowMapRenderer->bindTexture(ctx);

	reflectionMapRenderer->render(ctx, registry, skyboxRenderer);
	waterMapRenderer->render(ctx, registry, skyboxRenderer);

	drawMain(ctx);
	drawMirror(ctx);
	//drawScene(ctx);
	drawViewports(ctx);
}

void Scene::drawMain(RenderContext& ctx)
{
	RenderContext mainCtx(ctx.assets, ctx.clock, ctx.state, ctx.scene, ctx.camera, mainBuffer->spec.width, mainBuffer->spec.height);
	mainCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;

	mainBuffer->bind(mainCtx);
	drawScene(mainCtx);
	mainBuffer->unbind(ctx);
}

// "back mirror" viewport
void Scene::drawMirror(RenderContext& ctx)
{
	if (!showMirrorView) return;

	Camera camera(ctx.camera->getPos(), ctx.camera->getFront(), ctx.camera->getUp());
	camera.setZoom(ctx.camera->getZoom());

	glm::vec3 rot = ctx.camera->getRotation();
	//rot.y += 180;
	rot.y += 180;
	camera.setRotation(-rot);

	RenderContext mirrorCtx(ctx.assets, ctx.clock, ctx.state, ctx.scene, &camera, mirrorBuffer->spec.width, mirrorBuffer->spec.height);
	mirrorCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;
	mirrorCtx.bindMatricesUBO();

	mirrorBuffer->bind(mirrorCtx);

	drawScene(mirrorCtx);

	mirrorBuffer->unbind(ctx);
	ctx.bindMatricesUBO();
}

void Scene::drawViewports(RenderContext& ctx)
{
	ctx.state.disable(GL_DEPTH_TEST);
	ctx.state.enable(GL_BLEND);
	viewportRenderer->render(ctx, registry);
	ctx.state.disable(GL_BLEND);
}

void Scene::drawScene(RenderContext& ctx)
{
	glClearColor(0.9f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	reflectionMapRenderer->bindTexture(ctx);
	waterMapRenderer->bindTexture(ctx);

	if (skyboxRenderer) {
		skyboxRenderer->render(ctx, registry);
	}

	//ctx.state.enable(GL_CLIP_DISTANCE0);
	//ClipPlaneUBO& clip = ctx.clipPlanes.clipping[0];
	//clip.enabled = true;
	//clip.plane = glm::vec4(0, -1, 0, 15);
	//ctx.bindClipPlanesUBO();

	nodeRenderer->render(ctx, registry);

	//clip.enabled = false;
	//ctx.bindClipPlanesUBO();
	//ctx.state.disable(GL_CLIP_DISTANCE0);

	particleSystem->render(ctx);

	if (showNormals) {
		normalRenderer->render(ctx, registry);
	}
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

void Scene::bindComponents(Node* node)
{
	if (node->camera) {
		cameraNode = node;
	}

	Light* light = node->light;
	if (light) {
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

	if (node->particleGenerator) {
		node->particleGenerator->system = particleSystem;
		particleGenerators.push_back(node->particleGenerator);
	}
}

void Scene::updateMainViewport(RenderContext& ctx)
{
	if (!mainBuffer) {
		mainBuffer = new TextureBuffer({ ctx.width, ctx.height });
		mainBuffer->prepare();
		mainViewport->setTextureID(mainBuffer->textureID);
	}
}

void Scene::prepareUBOs()
{
	// Matrices
	{
		int sz = sizeof(MatricesUBO);

		glCreateBuffers(1, &ubo.matrices);
		glNamedBufferStorage(ubo.matrices, sz, nullptr, GL_DYNAMIC_STORAGE_BIT);

		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATRICES, ubo.matrices, 0, sz);
		ubo.matricesSize = sz;
	}
	// Data
	{
		int sz = sizeof(DataUBO);

		glCreateBuffers(1, &ubo.data);
		glNamedBufferStorage(ubo.data, sz, nullptr, GL_DYNAMIC_STORAGE_BIT);

		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_DATA, ubo.data, 0, sz);
		ubo.dataSize = sz;
	}
	// Clipping
	{
		int sz = sizeof(ClipPlanesUBO);

		glCreateBuffers(1, &ubo.clipPlanes);
		glNamedBufferStorage(ubo.clipPlanes, sz, nullptr, GL_DYNAMIC_STORAGE_BIT);

		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_CLIP_PLANES, ubo.clipPlanes, 0, sz);
		ubo.clipPlanesSize = sz;
	}
	// Lights
	{
		int sz = sizeof(LightsUBO);

		glCreateBuffers(1, &ubo.lights);
		glNamedBufferStorage(ubo.lights, sz, nullptr, GL_DYNAMIC_STORAGE_BIT);

		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_LIGHTS, ubo.lights, 0, sz);
		ubo.lightsSize = sz;
	}
}

#include "ReflectionMapRenderer.h"

#include <vector>

#include "SkyboxRenderer.h"



ReflectionMapRenderer::ReflectionMapRenderer(const Assets& assets)
	: Renderer(assets)
{
}

ReflectionMapRenderer::~ReflectionMapRenderer()
{
	delete reflectionMap;
}

void ReflectionMapRenderer::prepare()
{
	reflectionMap = new DynamicCubeMap(assets.reflectionCubeSize);
	reflectionMap->prepare();
}

void ReflectionMapRenderer::bind(const RenderContext& ctx)
{
}

void ReflectionMapRenderer::bindTexture(const RenderContext& ctx)
{
	if (!rendered) return;
	reflectionMap->bindTexture(ctx, assets.reflectionMapUnitIndex);
}

void ReflectionMapRenderer::render(const RenderContext& mainCtx, NodeRegistry& registry, SkyboxRenderer* skybox)
{
	if (drawIndex++ < drawSkip) return;
	drawIndex = 0;

	Node* centerNode = findCenter(mainCtx, registry);
	if (!centerNode) return;

	// https://www.youtube.com/watch?v=lW_iqrtJORc
	// https://eng.libretexts.org/Bookshelves/Computer_Science/Book%3A_Introduction_to_Computer_Graphics_(Eck)/07%3A_3D_Graphics_with_WebGL/7.04%3A_Framebuffers
	// view-source:math.hws.edu/eck/cs424/graphicsbook2018/source/webgl/cube-camera.html

	reflectionMap->bind(mainCtx);

	// +X (right)
	// -X (left)
	// +Y (top)
	// -Y (bottom)
	// +Z (front) 
	// -Z (back)
	glm::vec3 cameraFront[6] = {
		{  1,  0,  0 },
		{ -1,  0,  0 },
		{  0,  1,  0 },
		{  0, -1,  0 },
		{  0,  0,  1 },
		{  0,  0, -1 },
	};
	glm::vec3 cameraUp[6] = {
		{  0, -1,  0 },
		{  0, -1,  0 },
		{  0,  0,  1 },
		{  0,  0, -1 },
		{  0, -1,  0 },
		{  0, -1,  0 },
	};

	const glm::vec3& center = centerNode->getPos();

	for (int i = 0; i < 6; i++) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, reflectionMap->textureID, 0);
		//glClearColor(0.9f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Camera camera(center, cameraFront[i], cameraUp[i]);
		camera.setZoom(90);
		RenderContext ctx(mainCtx.assets, mainCtx.clock, mainCtx.state, mainCtx.scene, &camera, reflectionMap->size, reflectionMap->size);
		ctx.lightSpaceMatrix = mainCtx.lightSpaceMatrix;
		ctx.bindMatricesUBO();

		skybox->render(ctx, registry);
		drawNodes(ctx, registry);
	}

//	bindTexture(mainCtx);
//	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	reflectionMap->unbind(mainCtx);

	rendered = true;

}

void ReflectionMapRenderer::drawNodes(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto& x : registry.nodes) {
		NodeType* t = x.first;
		Shader* shader = t->bind(ctx, nullptr);
		//shader->hasReflectionMap.set(false);

		Batch& batch = t->batch;
		batch.bind(ctx, shader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, shader);
		}

		batch.flush(ctx, t);
		t->unbind(ctx);
	}
}

Node* ReflectionMapRenderer::findCenter(const RenderContext& ctx, NodeRegistry& registry)
{
	const glm::vec3& cameraPos = ctx.camera->getPos();
	const glm::vec3& cameraDir = ctx.camera->getViewFront();

	std::map<float, Node*> sorted;
	for (auto& x : registry.nodes) {
		if (!x.first->hasReflection()) continue;

		for (auto& e : x.second) {
			glm::vec3 ray = e->getPos() - cameraPos;
			float distance = glm::length(ray);
			glm::vec3 fromCamera = glm::normalize(ray);
			float dot = glm::dot(fromCamera, cameraDir);
			if (dot < 0) continue;
			sorted[dot] = e;
		}
	}

	for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
		return it->second;
	}
	return nullptr;
}

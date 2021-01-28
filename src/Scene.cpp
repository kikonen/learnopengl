#include "Scene.h"

#include "KIGL.h"

const unsigned int SHADOW_WIDTH = 1024,
	SHADOW_HEIGHT = 1024;

unsigned int quadVAO = 0;
unsigned int quadVBO;
void drawQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, 0.5f, 0.0f, 0.0f, 0.0f,
			-0.5f, 1.0f, 0.0f, 1.0f, 1.0f,
			-0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}


Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::prepare()
{
	// NOTE KI OpenGL does NOT like interleaved draw and prepare
	for (auto node : nodes) {
		node->prepare(nullptr);
		node->prepare(stencilShader);
		node->prepare(shadowShader);
		if (showNormals) {
			node->prepare(normalShader);
		}
	}

	prepareShadowMap();
}

void Scene::bind(RenderContext& ctx)
{
	bindShadowMap(ctx);
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

	drawShadowMap(ctx);

	glActiveTexture(ctx.engine.assets.depthMapUnitId);
	glBindTexture(GL_TEXTURE_2D, shadowMap);

	drawScene(ctx);

	drawNormals(ctx);
	drawDebugShadowMap(ctx);

	KIGL::checkErrors("scene.draw");
}

void Scene::drawScene(RenderContext& ctx)
{
	drawSelected(ctx);
	drawNodes(ctx);
	drawSelectedStencil(ctx);
}

void Scene::drawNormals(RenderContext& ctx)
{
	if (!showNormals) {
		return;
	}

	for (auto node : nodes) {
		node->bind(ctx, normalShader);
		node->draw(ctx);
	}
}

// draw all non selected nodes
void Scene::drawNodes(RenderContext& ctx)
{
	glStencilMask(0x00);
	std::vector<Node*> blendedNodes;
	for (auto node : nodes) {
		if (std::find(selection.begin(), selection.end(), node) != selection.end()) {
			continue;
		}
		if (node->blend) {
			blendedNodes.push_back(node);
		}
		else {
			node->bind(ctx, nullptr);
			node->mesh->bound->shader->setInt("shadowMap", ctx.engine.assets.depthMapUnitIndex);
			node->draw(ctx);
		}
	}

	if (skybox) {
		skybox->draw(ctx);
	}
	drawBlended(blendedNodes, ctx);
}

// draw all selected nodes for stencil
void Scene::drawSelected(RenderContext& ctx)
{
	if (selection.empty()) {
		return;
	}

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	std::vector<Node*> blendedNodes;
	for (auto node : selection) {
		if (node->blend) {
			blendedNodes.push_back(node);
		}
		else {
			node->bind(ctx, nullptr);
			node->mesh->bound->shader->setInt("shadowMap", ctx.engine.assets.depthMapUnitIndex);
			node->draw(ctx);
		}
	}

	drawBlended(blendedNodes, ctx);
}

// draw all selected nodes with stencil
void Scene::drawSelectedStencil(RenderContext& ctx)
{
	if (selection.empty()) {
		return;
	}

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDisable(GL_DEPTH_TEST);

	for (auto node : selection) {
		float scale = node->getScale();
		node->setScale(scale * 1.02f);
		node->bind(ctx, stencilShader);
		node->draw(ctx);
		node->setScale(scale);
	}

	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glEnable(GL_DEPTH_TEST);
}

void Scene::drawBlended(std::vector<Node*>& nodes, RenderContext& ctx)
{
	if (nodes.empty()) {
		return;
	}

	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	// TODO KI discards nodes if *same* distance
	std::map<float, Node*> sorted;
	for (auto node : nodes) {
		float distance = glm::length(ctx.engine.camera.getPos() - node->getPos());
		sorted[distance] = node;
	}

	for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
		Node* node = it->second;
		node->bind(ctx, nullptr);
		node->mesh->bound->shader->setInt("shadowMap", ctx.engine.assets.depthMapUnitIndex);
		node->draw(ctx);
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void Scene::prepareShadowMap()
{
	glGenFramebuffers(1, &shadowMapFBO);

	// depth map
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shadowShader->setup();
	shadowDebugShader->setup();
}

void Scene::bindShadowMap(RenderContext& ctx)
{
	glm::mat4 b = {
		{0.5f, 0.0f, 0.0f, 0.0f},
		{0.0f, 0.5f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.5f, 0.0f},
		{0.5f, 0.5f, 0.5f, 1.0f},
	};

	//glm::vec3 lightPos = { 30.f, 30.f, -30.f };
	//glm::vec3 lightPos = { 10.f, 30.f, -20.f };
	glm::vec3 lightPos = { 5.f, 30.f, -10.f };
	//lightPos = ctx.engine.camera.getPos();

	const float radius = 15.0f;
	float posX = sin(glfwGetTime() / 8) * radius;
	//float posY = sin(glfwGetTime() / 16) * radius
	float posZ = cos(glfwGetTime() / 8) * radius;

	lightPos = glm::vec3(posX, 10, posZ) + groundOffset;
	//lightPos = glm::vec3(10, 7, 0) + groundOffset;

	glm::vec3 target = glm::vec3(0.0f) + groundOffset;
	//target = lightPos + glm::vec3(1, 0, 0);
	glm::mat4 lightView = glm::lookAt(lightPos, target, glm::vec3(0.0, 1.0, 0.0));

	float near_plane = 0.1f, far_plane = 100.5f;
	glm::mat4 lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);

	//lightProjection = glm::perspective(glm::radians(60.0f), (float)ctx.engine.width / (float)ctx.engine.height, near_plane, far_plane);

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	dirLight->pos = lightPos;
	dirLight->dir = glm::normalize(target - lightPos);

	ctx.lightSpaceMatrix = lightSpaceMatrix;
}

void Scene::drawShadowMap(RenderContext& ctx)
{
	// bind
	// 1. first render to depth map
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	drawShadow(ctx);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// reset viewport
	glViewport(0, 0, ctx.engine.width, ctx.engine.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Scene::drawShadow(RenderContext& ctx)
{
	//glCullFace(GL_FRONT);

	std::vector<Node*> blendedNodes;
	for (auto node : nodes) {
		if (node->light || node->skipShadow) {
			continue;
		}
		if (node->blend) {
			blendedNodes.push_back(node);
		}
		else {
			node->bind(ctx, shadowShader);
			node->draw(ctx);
		}
	}

	drawBlendedShadow(blendedNodes, ctx);

	//glCullFace(GL_BACK); 
}

void Scene::drawBlendedShadow(std::vector<Node*>& nodes, RenderContext& ctx)
{
	if (nodes.empty()) {
		return;
	}

	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	// TODO KI discards nodes if *same* distance
	std::map<float, Node*> sorted;
	for (auto node : nodes) {
		float distance = glm::length(ctx.engine.camera.getPos() - node->getPos());
		sorted[distance] = node;
	}

	for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
		Node* node = it->second;
		node->bind(ctx, shadowShader);
		node->draw(ctx);
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void Scene::drawDebugShadowMap(RenderContext& ctx)
{
	Shader* shader = shadowDebugShader;
	shader->use();
	shader->setInt("shadowMap", ctx.engine.assets.depthMapUnitIndex);

	glActiveTexture(ctx.engine.assets.depthMapUnitId);
	glBindTexture(GL_TEXTURE_2D, shadowMap);

	float near_plane = 0.1f, far_plane = 100.5f;

	shader->setFloat("nearPlane", near_plane);
	shader->setFloat("farPlane", far_plane);

	drawQuad();
}


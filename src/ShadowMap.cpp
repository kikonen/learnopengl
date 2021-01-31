#include "ShadowMap.h"

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


ShadowMapRenderer::ShadowMapRenderer(const Assets& assets)
	: assets(assets)
{
	shadowShader = Shader::getShader(assets, TEX_SIMPLE_DEPTH, "");
	shadowDebugShader = Shader::getShader(assets, TEX_DEBUG_DEPTH, "");
}


void ShadowMapRenderer::prepare()
{
	glGenFramebuffers(1, &shadowMapFBO);

	// depth map
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glm::vec4 borderColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(borderColor));

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shadowShader->setup();
	shadowDebugShader->setup();
}

void ShadowMapRenderer::bind(RenderContext& ctx)
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

	lightPos = glm::vec3(posX, 10, posZ) + assets.groundOffset;
	//lightPos = glm::vec3(10, 7, 0) + groundOffset;

	glm::vec3 target = glm::vec3(0.0f) + assets.groundOffset;
	//target = lightPos + glm::vec3(1, 0, 0);
	glm::mat4 lightView = glm::lookAt(lightPos, target, glm::vec3(0.0, 1.0, 0.0));

	float near_plane = 0.1f, far_plane = 1000.0f;
	glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);

	//lightProjection = glm::perspective(glm::radians(60.0f), (float)ctx.engine.width / (float)ctx.engine.height, near_plane, far_plane);

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	this->lightPos = lightPos;
	this->lightDir = glm::normalize(target - lightPos);

	ctx.lightSpaceMatrix = lightSpaceMatrix;
}

void ShadowMapRenderer::draw(RenderContext& ctx, std::vector<Node*>& nodes)
{
	for (auto node : nodes) {
		node->prepare(shadowShader);
	}

	// bind
	// 1. first render to depth map
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

	glClear(GL_DEPTH_BUFFER_BIT);

	drawNodes(ctx, nodes);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// reset viewport
	glViewport(0, 0, ctx.engine.width, ctx.engine.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void ShadowMapRenderer::drawNodes(RenderContext& ctx, std::vector<Node*>& nodes)
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

	drawBlendedNodes(blendedNodes, ctx);

	//glCullFace(GL_BACK); 
}

void ShadowMapRenderer::drawBlendedNodes(std::vector<Node*>& nodes, RenderContext& ctx)
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

void ShadowMapRenderer::drawDebug(RenderContext& ctx)
{
	Shader* shader = shadowDebugShader;
	shader->bind();
	shader->shadowMap.set(ctx.engine.assets.shadowMapUnitIndex);

	glActiveTexture(ctx.engine.assets.shadowMapUnitId);
	glBindTexture(GL_TEXTURE_2D, shadowMap);

	float near_plane = 0.1f, far_plane = 1000.0f;


	shader->nearPlane.set(near_plane);
	shader->farPlane.set(far_plane);

	drawQuad();
}


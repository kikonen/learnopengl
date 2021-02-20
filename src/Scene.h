#pragma once

#include <vector>
#include <string>
#include <functional>
#include <mutex>

#include "NodeRegistry.h"
#include "Light.h"
#include "RenderContext.h"
#include "Node.h"
#include "Terrain.h"
#include "Viewport.h"

#include "NodeRenderer.h"
#include "SpriteRenderer.h"
#include "TerrainRenderer.h"

#include "ViewportRenderer.h"
#include "SkyboxRenderer.h"
#include "ShadowMapRenderer.h"
#include "ReflectionMapRenderer.h"
#include "NormalRenderer.h"

#include "ParticleSystem.h"

#include "TextureBuffer.h"

class Scene final
{
public:
	Scene(const Assets& assets);
	~Scene();

	void prepare();

	void processEvents(RenderContext& ctx);
	void update(RenderContext& ctx);
	void bind(RenderContext& ctx);
	void draw(RenderContext& ctx);

	Camera* getCamera();
	Node* getCameraNode();

	Light* getDirLight();
	std::vector<Light*>& getPointLights();
	std::vector<Light*>& getSpotLights();

	void addCamera(Node* node);
	void addLight(Node* node);

private:
	void prepareUBOs();

public:
	const Assets& assets;

	bool showNormals = false;

	SkyboxRenderer* skyboxRenderer = nullptr;
	UBO ubo;

	NodeRegistry registry;

protected:

private:
	Node* cameraNode = nullptr;

	Light* dirLight = nullptr;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;

	NodeRenderer* nodeRenderer = nullptr;
	SpriteRenderer* spriteRenderer = nullptr;

	TerrainRenderer* terrainRenderer = nullptr;
	ViewportRenderer* viewportRenderer = nullptr;

	ShadowMapRenderer* shadowMapRenderer = nullptr;
	ReflectionMapRenderer* reflectionMapRenderer = nullptr;
	NormalRenderer* normalRenderer = nullptr;

	ParticleSystem* particleSystem = nullptr;

	bool useMirrorView = false;
	TextureBuffer* framebuffer = nullptr;
	Viewport* frameViewport = nullptr;
	Shader* viewportShader = nullptr;
};

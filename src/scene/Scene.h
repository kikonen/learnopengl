#pragma once

#include <vector>
#include <string>
#include <functional>
#include <mutex>

#include "model/Node.h"
#include "component/Light.h"
#include "NodeRegistry.h"
#include "RenderContext.h"

#include "renderer/NodeRenderer.h"
#include "renderer/SpriteRenderer.h"
#include "renderer/TerrainRenderer.h"

#include "renderer/ViewportRenderer.h"
#include "renderer/SkyboxRenderer.h"
#include "renderer/ShadowMapRenderer.h"
#include "renderer/ReflectionMapRenderer.h"
#include "renderer/NormalRenderer.h"

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

	void bindComponents(Node* node);

private:
	void prepareUBOs();

public:
	const Assets& assets;

	bool showNormals = false;
	bool showMirrorView = false;

	SkyboxRenderer* skyboxRenderer = nullptr;
	UBO ubo;

	NodeRegistry registry;

protected:

private:
	Node* cameraNode = nullptr;

	Light* dirLight = nullptr;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;
	std::vector<ParticleGenerator*> particleGenerators;

	NodeRenderer* nodeRenderer = nullptr;
	SpriteRenderer* spriteRenderer = nullptr;

	TerrainRenderer* terrainRenderer = nullptr;
	ViewportRenderer* viewportRenderer = nullptr;

	ShadowMapRenderer* shadowMapRenderer = nullptr;
	ReflectionMapRenderer* reflectionMapRenderer = nullptr;
	NormalRenderer* normalRenderer = nullptr;

	ParticleSystem* particleSystem = nullptr;

	TextureBuffer* mirrorBuffer = nullptr;
	Viewport* mirrorViewport = nullptr;
};

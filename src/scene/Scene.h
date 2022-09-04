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
//#include "renderer/TerrainRenderer.h"
#include "renderer/ViewportRenderer.h"

#include "renderer/WaterMapRenderer.h"
#include "renderer/CubeMapRenderer.h"
#include "renderer/ShadowMapRenderer.h"

#include "renderer/SkyboxRenderer.h"

#include "renderer/ObjectIdRenderer.h"
#include "renderer/NormalRenderer.h"

#include "ParticleSystem.h"

#include "TextureBuffer.h"

class Scene final
{
public:
	Scene(const Assets& assets);
	~Scene();

	void prepare();

	void attachNodes();

	void processEvents(RenderContext& ctx);
	void update(RenderContext& ctx);
	void bind(RenderContext& ctx);

	void draw(RenderContext& ctx);

	void drawMain(RenderContext& ctx);
	void drawMirror(RenderContext& ctx);
	void drawViewports(RenderContext& ctx);

	void drawScene(RenderContext& ctx);

	Camera* getCamera();
	Node* getCameraNode();

	Light* getDirLight();
	std::vector<Light*>& getPointLights();
	std::vector<Light*>& getSpotLights();

	void bindComponents(Node* node);
	int getObjectID(const RenderContext& ctx, double posx, double posy);

private:
	void updateMainViewport(RenderContext& ctx);
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

	//TerrainRenderer* terrainRenderer = nullptr;
	ViewportRenderer* viewportRenderer = nullptr;

	WaterMapRenderer* waterMapRenderer = nullptr;
	CubeMapRenderer* cubeMapRenderer = nullptr;
	ShadowMapRenderer* shadowMapRenderer = nullptr;

	ObjectIdRenderer* objectIdRenderer = nullptr;
	NormalRenderer* normalRenderer = nullptr;

	ParticleSystem* particleSystem = nullptr;

	TextureBuffer* mirrorBuffer = nullptr;
	Viewport* mirrorViewport = nullptr;

	TextureBuffer* mainBuffer = nullptr;
	Viewport* mainViewport = nullptr;

	unsigned int pbo = -1;
};

#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "Engine.h"
#include "Shader.h"
#include "ShaderInfo.h"
#include "RenderContext.h"

#include "Tri.h"
#include "Material.h"
#include "Shader.h"
#include "Camera.h"
#include "Vertex.h"
#include "Light.h"
#include "Assets.h"

class ModelMesh {
public:
	ModelMesh(
		const std::string& modelName);

	ModelMesh(
		const std::string& modelName,
		const std::string& path);

	~ModelMesh();

	ShaderInfo* prepare(Shader* shader);
	ShaderInfo* bind(const RenderContext& ctx, Shader* shader);
	void draw(const RenderContext& ctx);
	void drawInstanced(const RenderContext& ctx, int instanceCount);

	int	load(Engine& engine);

private:
	ShaderInfo* prepareShader(Shader* shader);
public:
	bool useWireframe = false;

	ShaderInfo* bound = nullptr;

	Shader* defaultShader = nullptr;
	Material* defaultMaterial = nullptr;

	unsigned int textureCount = 0;

private:
	std::map<std::string, ShaderInfo*> shaders;

	const std::string path = { "/" };
	const std::string modelName = { "" };

	std::vector<Tri> tris;
	std::vector<Vertex> vertexes;

	std::map<std::string, Material*> materials;

	bool hasTexture = false;
};

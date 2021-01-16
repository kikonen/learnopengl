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
		const Engine& engine,
		const std::string& modelName,
		const std::string& shaderName);

	ModelMesh(
		const Engine& engine, 
		const std::string& path,
		const std::string& modelName,
		const std::string& shaderName);

	~ModelMesh();

	ShaderInfo* prepare(Shader* shader);
	int bind(const RenderContext& ctx, Shader* shader);
	int draw(const RenderContext& ctx);
	int drawInstanced(const RenderContext& ctx, int instanceCount);

	int load();

private:
	ShaderInfo* prepareShader(Shader* shader);
public:
	bool useWireframe = false;
	bool debugColors = false;

	std::string name;

	const std::string shaderName;
	std::string geometryType;

	ShaderInfo* bound = nullptr;

	Material* defaultMaterial;
	bool overrideMaterials;

	unsigned int textureCount = 0;

private:
	const Engine& engine;

	std::map<std::string, ShaderInfo*> shaders;

	const std::string path;
	const std::string modelName;

	std::vector<Tri> tris;
	std::vector<Vertex> vertexes;

	std::map<std::string, Material*> materials;

	bool hasTexture = false;
};

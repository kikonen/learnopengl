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

	ShaderInfo* prepare(bool stencil);
	int bind(const RenderContext& ctx, bool stencil);
	int draw(const RenderContext& ctx);

	int load();

private:
	ShaderInfo* prepareShader(Shader* shader, bool stencil);
public:
	bool useTexture = true;
	bool useWireframe = false;
	bool debugColors = false;

	std::string textureType = TEX_TEXTURE;
	std::string stencilType = TEX_STENCIL;
	std::string geometryType = GEOM_NONE;

	std::string name;
	
	ShaderInfo* bound = nullptr;

	Material* defaultMaterial;
	bool overrideMaterials;

	unsigned int textureCount = 0;

private:
	const Engine& engine;

	std::map<bool, ShaderInfo*> shaders;

	const std::string path;
	const std::string modelName;
	const std::string shaderName;

	std::vector<Tri> tris;
	std::vector<Vertex> vertexes;

	std::map<std::string, Material*> materials;

	bool hasTexture = false;
 };
